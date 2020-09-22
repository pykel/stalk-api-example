#include "route_logging.h"
#include <string>
#include <functional>
#include <boost/asio.hpp>
#include "nlohmann/json.hpp"
#include "stalk/stalk_server.h"
#include "logger/logger.h"
#include "middleware/middleware.h"
#include "middleware/middleware_metrics.h"
#include "middleware/middleware_delayer.h"
#include "middleware/middleware_secure_header_hoister.h"
#include "metrics_instance.h"
#include "routes/route_chains.h"

using json = nlohmann::json;

namespace {

std::shared_ptr<spdlog::logger> logger()
{
    static std::shared_ptr<spdlog::logger> l;
    if (!l)
    {
        l = Logger::get("route-logging");
    }
    return l;
}

}


RouteLogging::RouteLogging(std::shared_ptr<Stalk::WebServer> server)
{
    auto logger = Logger::get("route-logging");
    logger->info("RouteLogging()");

    static const std::string path = "/logging";


    auto chain = Routes::buildRouteChainBasic(path);

    auto handleGet = [logger](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> /*chain*/)
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

    // logging 'GET' route
    auto route = Stalk::Route::Http(
        path,
        { Stalk::Verb::Get },
        [chain, logger](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
            {
                logger->info("Received {}", req);

                chain->add([](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> /*chain*/) {
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
                });

                Middleware::Session session(std::move(detail), std::move(req), std::move(variables), Stalk::Response(), std::move(send));
                chain->process(std::move(session));
            }
    );

    route.setAcceptLongerPaths();
    server->addHttpRoute(std::move(route));
}
