#include "core_routes.h"
#include <string>
#include <functional>
#include <boost/asio.hpp>
#include "stalk/stalk_websocket_client.h"
#include "stalk/stalk_server.h"
#include "logger/logger.h"
#include "middleware/middleware.h"
#include "middleware/middleware_metrics.h"
#include "middleware/middleware_delayer.h"
#include "middleware/middleware_secure_header_hoister.h"
#include "metrics_instance.h"
#include "routes/route_chains.h"
#include "routes/route_logging.h"
#include "routes/route_metrics.h"
#include "routes/route_delayer.h"


CoreRoutes::CoreRoutes(boost::asio::io_context& ioc, std::shared_ptr<Stalk::WebServer> server) :
    ioc_(ioc),
    server_(server)
{
    auto logger = Logger::get("core-routes");
    logger->info("CoreRoutes()");

    RouteLogging { server };
    RouteMetrics { server };
    RouteDelayer { ioc, server };

    {
        const std::string path = "/";

        auto route = Stalk::Route::Http(
            path,
            { Stalk::Verb::Get, Stalk::Verb::Post },
            [logger, path](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
                {
                    logger->info("Received {}", req);

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

    server_->addWebsocketRoute(Stalk::Route::Websocket(
        "/ws",
        Stalk::RoutedWebsocketPreUpgradeCb(),
        [logger](bool connected, std::shared_ptr<Stalk::WebsocketSession> session, Stalk::RequestVariables&& variables) {
            logger->info("WebSocket Route Callback: Connected:{}", connected);

            session->send("Hello websocket msg from Stalk");
        },
        [logger](std::shared_ptr<Stalk::WebsocketSession> session, std::string&& msg) {
            logger->info("WebSocket Route Callback: msg:{}", msg);
        }));

    logger->trace("CoreRoutes() exit");
}
