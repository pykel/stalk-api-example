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
#include "routes/route_helpers.h"


CoreRoutes::CoreRoutes(boost::asio::io_context& ioc, std::shared_ptr<Stalk::WebServer> server) :
    ioc_(ioc),
    server_(server)
{
    auto logger = Logger::get("core-routes");
    logger->info("CoreRoutes()");

    RouteLogging { server };
    RouteMetrics { server };
    RouteDelayer { ioc, server };

    // Add a basic route with no middleware chain

    {
        const std::string path = "/raw";

        auto route = Stalk::Route::Http(
            path,
            { Stalk::Verb::Get, Stalk::Verb::Put, Stalk::Verb::Delete, Stalk::Verb::Post },
            [logger](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
                {
                    logger->info("Received request: {} {}", req.target(), Stalk::verbString(req.method()));

                    auto resp = Stalk::Response(req).status(Stalk::Status::ok);

                    logger->info("Sending response: {} body-size:{}", resp.status(), resp.body().size());

                    send(std::move(resp));
                });
        route.setAcceptLongerPaths();
        server_->addHttpRoute(std::move(route));
    }

    // Add a simple route that uses the basic middleware chain

    {
        const std::string path = "/";

        auto chain = Routes::buildRouteChainBasic(path);

        auto chainFn = [logger](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> /*chain*/)
            {
                logger->info("Received {}", session.req);

                auto resp = Stalk::Response();
                session.send(std::move(resp));
            };

        Routes::addRouteWithChain(server, path, { Stalk::Verb::Get, Stalk::Verb::Post }, chain, chainFn);
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
