#include "middleware_metrics.h"
#include <functional>

namespace Middleware
{


Metrics::Metrics(const std::string& name, const std::map<std::string, std::string>& labels) :
    counterFamily_(prometheus::BuildCounter().Name(name)
                                       .Help("Middleware Metrics")
                                       .Labels(labels)
                                       .Register(::Metrics::registry()))
{
#if 0
# HELP http_requests_total The total number of HTTP requests.
# TYPE http_requests_total counter
http_requests_total{method="post",code="200"} 1027 1395066363000
http_requests_total{method="post",code="400"}    3 1395066363000
#endif
    logger = Logger::get("Middleware.Metrics" + std::to_string((long)this));
}

void Metrics::process(const std::string& path, Session&& session, std::shared_ptr<Chain> chain)
{
    logger->info("Processing {}", session.req);

    auto self = shared_from_this();

    // Replace the send function with one that increments the counter based on the response status code.

    Stalk::Verb method = session.req.method();
    Stalk::SendResponse metricSend = [self, this, path, method, send = std::move(session.send)](Stalk::Response&& resp)
        {
            auto status = resp.status();
            CounterKey key(path, method, resp.status());

            auto it = counters_.find(key);
            if (it == counters_.end())
            {
                auto& counter = counterFamily_.Add({ { "path", path }, { "method", std::string(Stalk::verbString(method)) }, { "code", std::to_string(static_cast<int>(resp.status())) } });
                bool inserted;
                std::tie(it, inserted) = counters_.try_emplace(key, counter);
            }

            it->second.Increment();

            send(std::move(resp));
        };

    session.send = std::move(metricSend);

    chain->process(std::move(session));
}

//void Metrics::operator()(Session&& session, std::shared_ptr<Chain> chain)
//{
//    process(std::move(session), chain);
//}

} // namespace Middleware
