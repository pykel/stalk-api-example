#include "core_routes.h"
#include <string>
#include <functional>
#include <boost/asio.hpp>
#include "stalk/stalk_websocket_client.h"
#include "stalk/stalk_server.h"
#include "prometheus/text_serializer.h"
#include "logger/logger.h"
#include "middleware/middleware.h"
#include "middleware/middleware_metrics.h"
#include "middleware/middleware_delayer.h"
#include "middleware/middleware_secure_header_hoister.h"
#include "metrics_instance.h"


std::shared_ptr<spdlog::logger> logger()
{
    static std::shared_ptr<spdlog::logger> l;
    if (!l)
    {
        l = Logger::get("core-routes");
    }
    return l;
}

CoreRoutes::CoreRoutes(boost::asio::io_context& ioc, std::shared_ptr<Stalk::WebServer> server) :
    ioc_(ioc),
    server_(server)
{
    logger()->info("CoreRoutes()");

    auto hoister = std::make_shared<Middleware::SecureHeaderHoister>();
    auto delayer = std::make_shared<Middleware::Delayer>(ioc);
    auto metrics = std::make_shared<Middleware::Metrics>("http_requests");

    {
        std::string path = "/";

        auto route = Stalk::Route::Http(
            path,
            { Stalk::Verb::Get, Stalk::Verb::Post },
            [path, metrics, hoister, delayer](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
                {
                    logger()->info("Received {}", req);

                    // Create middleware chain
                    auto chain = std::make_shared<Middleware::Chain>();

                    chain->add([metrics, path](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> chain) {
                        metrics->process(path, std::move(session), chain);
                    });

                    chain->add(Middleware::SecureHeaderHoister());

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
            [path, metrics, hoister, delayer](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
                {
                    logger()->info("Received {}", req);

                    // Create middleware chain
                    auto chain = std::make_shared<Middleware::Chain>();

                    chain->add([metrics, path](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> chain) {
                        metrics->process(path, std::move(session), chain);
                    });

                    chain->add(Middleware::SecureHeaderHoister());

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
            [path, metrics](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
                {
                    logger()->info("Received {}", req);

                    // Create middleware chain
                    auto chain = std::make_shared<Middleware::Chain>();

                    chain->add([metrics, path](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> chain) {
                        metrics->process(path, std::move(session), chain);
                    });

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
