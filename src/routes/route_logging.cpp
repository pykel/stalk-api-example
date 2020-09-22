#include "route_logging.h"
#include <string>
#include <functional>
#include "nlohmann/json.hpp"
#include "stalk/stalk_server.h"
#include "logger/logger.h"
#include "routes/route_chains.h"
#include "routes/route_helpers.h"

using json = nlohmann::json;


RouteLogging::RouteLogging(std::shared_ptr<Stalk::WebServer> server)
{
    auto logger = Logger::get("route-logging");
    logger->info("RouteLogging()");

    static const std::string path = "/logging";

    auto chain = Routes::buildRouteChainBasic(path);

    // logging 'GET' route
    auto getChainFn = [logger](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> /*chain*/)
        {
            auto loggers = Logger::loggers();
            auto j = json();
            for (const auto& l : loggers)
            {
                j.push_back( json { { "name", l.first } } );
            }

            auto resp = Stalk::Response()
                    .body(j.dump())
                    .status(session.req.method() == Stalk::Verb::Post ? Stalk::Status::accepted : Stalk::Status::ok);
            session.send(std::move(resp));
        };

    Routes::addRouteWithChain(server, path, { Stalk::Verb::Get }, chain, getChainFn);

    // logging 'POST' route to configure level
    // /logging/
}
