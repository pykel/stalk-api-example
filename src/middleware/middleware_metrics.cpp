#include "middleware_metrics.h"
#include <functional>

namespace {


prometheus::Family<prometheus::Counter>& counterFamily()
{
    static prometheus::Family<prometheus::Counter>& family(prometheus::BuildCounter().Name("http_requests")
                                       .Help("Middleware Metrics")
                                       .Register(::Metrics::registry()));
    return family;
}

std::shared_ptr<Logger> logger()
{
    static std::shared_ptr<Logger> l = Logger::get("Middleware.Metrics");
    return l;
}

} // anon namespace


namespace Middleware
{

#if 0
Metrics::Metrics() :
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
#endif
#if 0
std::shared_ptr<Metrics> Metrics::instance()
{
    static std::shared_ptr<Metrics> inst;
    if (!inst)
    {
        inst = std::make_shared<Metrics>();
    }

    return inst;
}
#endif
void Metrics::operator()(Session &&session, std::shared_ptr<Chain> chain)
{
    logger()->info("Processing {}", session.req);

//    auto self = shared_from_this();

    // Replace the send function with one that increments the counter based on the response status code,
    // and then calls 'send'.

#if 0
    Stalk::SendResponse metricSend =
        [this, self,
         path = chain->key(),
         method = session.req.method(),
         send = std::move(session.send)](Stalk::Response&& resp)
        {
            CounterKey key(path, method, resp.status());

            {
                std::scoped_lock lock(mutex_);

                auto it = counters_.find(key);
                if (it == counters_.end())
                {
                    auto& counter = counterFamily_.Add({ { "path", path },
                                                         { "method", std::string(Stalk::verbString(method)) },
                                                         { "code", std::to_string(static_cast<int>(resp.status())) } });
                    bool inserted;
                    std::tie(it, inserted) = counters_.try_emplace(key, counter);
                }

                it->second.Increment();
            }

            send(std::move(resp));
        };
#endif
    Stalk::SendResponse metricSend =
        [path = chain->key(),
         method = session.req.method(),
         send = std::move(session.send)](Stalk::Response&& resp)
        {
            auto& counter = counterFamily().Add({ { "path", path },
                                                  { "method", std::string(Stalk::verbString(method)) },
                                                  { "code", std::to_string(static_cast<int>(resp.status())) } });
            counter.Increment();

            send(std::move(resp));
        };
    session.send = std::move(metricSend);

    chain->process(std::move(session));
}

} // namespace Middleware
