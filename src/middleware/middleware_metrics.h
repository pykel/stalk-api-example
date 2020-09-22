#pragma once

#include <vector>
#include <map>
#include <tuple>
#include <prometheus/counter.h>
#include "stalk/stalk_types.h"
#include "logger/logger.h"
#include "middleware/middleware.h"
#include "metrics_instance.h"

namespace Middleware
{

class Metrics
{
public:
#if 0
    Metrics() :
        counterFamily_(prometheus::BuildCounter().Name("http_requests")
                                           .Help("Middleware Metrics")
                                           .Register(::Metrics::registry())),
        logger_(Logger::get("Middleware.Metrics"))
    {
    #if 0
    # HELP http_requests_total The total number of HTTP requests.
    # TYPE http_requests_total counter
    http_requests_total{method="post",code="200"} 1027 1395066363000
    http_requests_total{method="post",code="400"}    3 1395066363000
    #endif
    }

    Metrics(const Metrics& rhs) :
        counterFamily_(rhs.counterFamily_),
        logger_(rhs.logger_)
    {
    }

    Metrics(Metrics&& rhs) = default;

    /// Get the singleton instance.
    static std::shared_ptr<Metrics> instance();
#endif
    //void process(const std::string& path, Session&& session, std::shared_ptr<Chain> chain);
    void operator()(Session&& session, std::shared_ptr<Chain> chain);

private:

//    prometheus::Family<prometheus::Counter>& counterFamily_;
//    std::shared_ptr<Logger> logger_;
};

} // namespace Middleware
