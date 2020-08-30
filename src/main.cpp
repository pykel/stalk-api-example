#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <cxxopts.hpp>
#include "LUrlParser.h"
#include "prometheus/text_serializer.h"
#include "stalk/stalk_websocket_client.h"
#include "stalk/stalk_server.h"
#include "logger/logger.h"
#include "server_credentials.h"
#include "middleware/middleware.h"
#include "middleware/middleware_metrics.h"
#include "middleware/middleware_delayer.h"
#include "middleware/middleware_secure_header_hoister.h"
#include "metrics_instance.h"

/*
 * Scenarios
 ** HTTP Routing
 ** Websocket Routing
 ** Request/Response metrics
 ** JWT Routing
 ** ConnectionDetails Routing
 ** Deferred Responses
 ** Deferred websocket upgrade from connectionDetails lookup routing
*/

int main(int argc, const char* argv[])
{
    cxxopts::Options options("stalk-api-example", "Example API implementation using stalk lib");
    options.add_options()
        ("a,listen-address", "Webserver listen address", cxxopts::value<std::string>()->default_value("::1"))
        ("p,listen-port", "Webserver listen port", cxxopts::value<uint16_t>())
        ("l,log-level", "Logging level: trace,debug,info,warning,error,critical,off", cxxopts::value<std::string>()->default_value("info"))
        ("h,help", "Print usage")
        ;

    const auto args = options.parse(argc, argv);

    if (args.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    const std::string logLevel = args.count("log-level") ? args["log-level"].as<std::string>() : "info";

    Logger::setLevel(spdlog::level::from_str(logLevel));

    auto logger = Logger::get("stalk-api-example");

    boost::asio::io_context ioc;

    const std::string address = args.count("listen-address") ? args["listen-address"].as<std::string>() : "::1";
    const uint16_t port = args.count("listen-port") ? args["listen-port"].as<uint16_t>() : uint16_t();

    logger->info("Starting WebServer on port {}", port);

    auto webServer = std::make_shared<Stalk::WebServer>(ioc, address, port, serverKey(), serverCert());
    webServer->run();

    logger->info("WebServer started, listening on port {}", webServer->port());

    auto hoister = std::make_shared<Middleware::SecureHeaderHoister>();
    auto delayer = std::make_shared<Middleware::Delayer>(ioc);
    auto metrics = std::make_shared<Middleware::Metrics>("http_requests");

    {
        std::string path = "/";

        auto route = Stalk::Route::Http(
            path,
            { Stalk::Verb::Get },
            [logger, path, metrics, hoister, delayer](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
                {
                    logger->info("Received {}", req);

                    // Create middleware chain
                    auto chain = std::make_shared<Middleware::Chain>();

                    chain->add([metrics, path](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> chain) {
                        metrics->process(path, std::move(session), chain);
                    });

                    chain->add(Middleware::SecureHeaderHoister());

                    chain->add([logger, delayer](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> chain) {
                        delayer->process(std::move(session), chain);
                    });

                    Middleware::Session session(std::move(detail), std::move(req), std::move(variables), Stalk::Response(), std::move(send));
                    chain->process(std::move(session));
                });
        route.setAcceptLongerPaths();
        webServer->addHttpRoute(std::move(route));
    }

    {
        std::string path = "/metrics";

        auto route = Stalk::Route::Http(
            path,
            { Stalk::Verb::Get },
            [logger, path](Stalk::ConnectionDetail /*detail*/, Stalk::Request&& req, Stalk::RequestVariables&& /*variables*/, Stalk::SendResponse&& send)
                {
                    logger->info("Received {}", req);

                    auto metrics = Metrics::registry().Collect();

                    prometheus::TextSerializer serialiser;
                    std::ostringstream oss;
                    serialiser.Serialize(oss, metrics);
                    auto resp = Stalk::Response().body(oss.str());
                    send(std::move(resp));
                });
        route.setAcceptLongerPaths();
        webServer->addHttpRoute(std::move(route));
    }


    webServer->addWebsocketRoute(Stalk::Route::Websocket(
        "/ws",
        Stalk::RoutedWebsocketPreUpgradeCb(),
        [logger](bool connected, std::shared_ptr<Stalk::WebsocketSession> session, Stalk::RequestVariables&& variables) {
            logger->info("WebSocket Route Callback: Connected:{}", connected);

            session->send("Hello websocket msg from Stalk");
        },
        [logger](std::shared_ptr<Stalk::WebsocketSession> session, std::string&& msg) {
            logger->info("WebSocket Route Callback: msg:{}", msg);
        }));

    ioc.run();

    return 0;
}
