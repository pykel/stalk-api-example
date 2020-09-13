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

class Metrics : public std::enable_shared_from_this<Metrics>
{
public:
    Metrics(const std::string& metricName, const std::map<std::string, std::string>& metricLabels = std::map<std::string, std::string>());

    void process(const std::string& path, Session&& session, std::shared_ptr<Chain> chain);
    //void operator()(Session&& session, std::shared_ptr<Chain> chain);

private:

    prometheus::Family<prometheus::Counter>& counterFamily_;
    using CounterKey = std::tuple<std::string, Stalk::Verb, Stalk::Status>;
    std::map<CounterKey, prometheus::Counter&> counters_;
    std::shared_ptr<Logger> logger;
};

} // namespace Middleware
