#include "route_metrics.h"
#include <string>
#include <sstream>
#include "stalk/stalk_server.h"
#include "logger/logger.h"
#include "prometheus/text_serializer.h"
#include "metrics_instance.h"
#include "routes/route_chains.h"
#include "routes/route_helpers.h"


RouteMetrics::RouteMetrics(std::shared_ptr<Stalk::WebServer> server)
{
    auto logger = Logger::get("route-metrics");
    logger->info("RouteMetrics()");

    static const std::string path = "/metrics";

    auto chain = Routes::buildRouteChainBasic(path);

    auto chainFn = [logger](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> /*chain*/)
        {
            logger->info("Received {}", session.req);

            auto metrics = Metrics::registry().Collect();
            prometheus::TextSerializer serialiser;
            std::ostringstream oss;
            serialiser.Serialize(oss, metrics);

            auto resp = Stalk::Response().body(oss.str());
            session.send(std::move(resp));
        };

    Routes::addRouteWithChain(server, path, { Stalk::Verb::Get }, chain, chainFn);
}
