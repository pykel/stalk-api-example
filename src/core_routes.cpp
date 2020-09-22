#include "core_routes.h"
#include <string>
#include <functional>
#include <boost/asio.hpp>
#include "nlohmann/json.hpp"
#include "stalk/stalk_websocket_client.h"
#include "stalk/stalk_server.h"
#include "prometheus/text_serializer.h"
#include "logger/logger.h"
#include "middleware/middleware.h"
#include "middleware/middleware_metrics.h"
#include "middleware/middleware_delayer.h"
#include "middleware/middleware_secure_header_hoister.h"
#include "metrics_instance.h"
#include "routes/route_chains.h"
#include "routes/route_logging.h"

using json = nlohmann::json;


namespace {

std::shared_ptr<spdlog::logger> logger()
{
    static std::shared_ptr<spdlog::logger> l;
    if (!l)
    {
        l = Logger::get("core-routes");
    }
    return l;
}

}


CoreRoutes::CoreRoutes(boost::asio::io_context& ioc, std::shared_ptr<Stalk::WebServer> server) :
    ioc_(ioc),
    server_(server)
{
    logger()->info("CoreRoutes()");

    {
        RouteLogging route(server);
    }

    {
        const std::string path = "/";

        auto route = Stalk::Route::Http(
            path,
            { Stalk::Verb::Get, Stalk::Verb::Post },
            [path](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
                {
                    logger()->info("Received {}", req);

                    // Create standard chain
                    auto chain = Routes::buildRouteChainBasic(path);

                    chain->add([](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> /*chain*/) {
                        auto resp = Stalk::Response()
                                .body(std::string("Hello"))
                                .status(session.req.method() == Stalk::Verb::Post ? Stalk::Status::accepted : Stalk::Status::ok);
                        session.send(std::move(resp));
                    });

                    Middleware::Session session(std::move(detail), std::move(req), std::move(variables), Stalk::Response(), std::move(send));
                    chain->process(std::move(session));
                });
        route.setAcceptLongerPaths();
        server_->addHttpRoute(std::move(route));
    }

    {
        std::string path = "/delay";

        auto route = Stalk::Route::Http(
            path,
            { Stalk::Verb::Get, Stalk::Verb::Post },
            [path, &ioc](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
                {
                    logger()->info("Received {}", req);

                    // Create standard chain
                    auto chain = Routes::buildRouteChainBasic(path);

                    // Add a delayer to the chain
                    auto delayer = std::make_shared<Middleware::Delayer>(ioc);

                    chain->add([delayer](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> chain) {
                        delayer->process(std::move(session), chain);
                    });

                    chain->add([](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> /*chain*/) {
                        auto resp = Stalk::Response()
                                .body(std::string("Delayed Reponse"))
                                .status(session.req.method() == Stalk::Verb::Post ? Stalk::Status::accepted : Stalk::Status::ok);
                        session.send(std::move(resp));
                    });

                    Middleware::Session session(std::move(detail), std::move(req), std::move(variables), Stalk::Response(), std::move(send));
                    chain->process(std::move(session));
                });
        route.setAcceptLongerPaths();
        server_->addHttpRoute(std::move(route));
    }

    {
        std::string path = "/metrics";

        auto route = Stalk::Route::Http(
            path,
            { Stalk::Verb::Get },
            [path](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
                {
                    logger()->info("Received {}", req);

                    // Create standard chain
                    auto chain = Routes::buildRouteChainBasic(path);

                    chain->add([](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> /*chain*/) {

                        auto metrics = Metrics::registry().Collect();
                        prometheus::TextSerializer serialiser;
                        std::ostringstream oss;
                        serialiser.Serialize(oss, metrics);

                        auto resp = Stalk::Response().body(oss.str());
                        session.send(std::move(resp));
                    });

                    Middleware::Session session(std::move(detail), std::move(req), std::move(variables), Stalk::Response(), std::move(send));
                    chain->process(std::move(session));
                });
        route.setAcceptLongerPaths();
        server_->addHttpRoute(std::move(route));
    }

    server_->addWebsocketRoute(Stalk::Route::Websocket(
        "/ws",
        Stalk::RoutedWebsocketPreUpgradeCb(),
        [](bool connected, std::shared_ptr<Stalk::WebsocketSession> session, Stalk::RequestVariables&& variables) {
            logger()->info("WebSocket Route Callback: Connected:{}", connected);

            session->send("Hello websocket msg from Stalk");
        },
        [](std::shared_ptr<Stalk::WebsocketSession> session, std::string&& msg) {
            logger()->info("WebSocket Route Callback: msg:{}", msg);
        }));

    logger()->info("CoreRoutes() exit");
}
