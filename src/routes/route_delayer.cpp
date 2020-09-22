#include "route_delayer.h"
#include <string>
#include <boost/asio.hpp>
#include "stalk/stalk_server.h"
#include "logger/logger.h"
#include "routes/route_chains.h"
#include "routes/route_helpers.h"
#include "middleware/middleware_delayer.h"


RouteDelayer::RouteDelayer(boost::asio::io_context& ioc, std::shared_ptr<Stalk::WebServer> server)
{
    auto logger = Logger::get("route-delayer");
    logger->info("RouteDelayer()");

    static const std::string path = "/delay";

    auto chain = Routes::buildRouteChainBasic(path);

    // Add a delayer to the standard middleware chain
    auto delayer = std::make_shared<Middleware::Delayer>(ioc);

    chain->add([delayer](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> chain) {
        delayer->operator()(std::move(session), chain);
    });

    auto chainFn = [](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> /*chain*/)
        {
            auto resp = Stalk::Response()
                    .body(std::string("Middleware Delayed Reponse"))
                    .status(session.req.method() == Stalk::Verb::Post ? Stalk::Status::accepted : Stalk::Status::ok);
            session.send(std::move(resp));
        };

    Routes::addRouteWithChain(server, path, { Stalk::Verb::Get, Stalk::Verb::Post }, chain, chainFn);
}
