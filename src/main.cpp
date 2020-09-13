#include <iostream>
#include <string>
#include <functional>
#include <boost/asio.hpp>
#include <cxxopts.hpp>
#include "stalk/stalk_server.h"
#include "stalk/stalk_logger.h"
#include "logger/logger.h"
#include "server_credentials.h"
#include "core_routes.h"

/*
 * Scenarios
 ** HTTP Routing
 ** Websocket Routing
 ** Middleware Chains - metrics/logging/role-based-security/x509-based-security
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

    const auto level = spdlog::level::from_str(logLevel);
    Logger::setLevel(level);

    auto logger = Logger::get("stalk-api-example");

    boost::asio::io_context ioc;

    const std::string address = args.count("listen-address") ? args["listen-address"].as<std::string>() : "::1";
    const uint16_t port = args.count("listen-port") ? args["listen-port"].as<uint16_t>() : uint16_t();

    logger->info("Starting WebServer on port {}", port);

    // Assign a logging callback to Stalk to hook into spdlog
    auto serverLogger = Logger::get("stalk");
    Stalk::Logger::setDefaultLevel(static_cast<Stalk::Logger::Level>(level));
    Stalk::Logger::setLogCb([serverLogger](Stalk::Logger::Level lvl, const std::string_view& msg)
        {
            // stalk levels same as spdlog level_enums
            serverLogger->log(static_cast<spdlog::level::level_enum>(lvl), msg);
        });

    auto webServer = std::make_shared<Stalk::WebServer>(ioc, address, port, serverKey(), serverCert());
    webServer->run();

    logger->info("WebServer started, listening on port {}", webServer->port());

    // Set up base routes
    CoreRoutes coreRoutes(ioc, webServer);

    try
    {
        ioc.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        throw;
    }

    return 0;
}
