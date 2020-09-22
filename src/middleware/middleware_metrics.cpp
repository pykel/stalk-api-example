#include "middleware_metrics.h"
#include <functional>
#include <prometheus/counter.h>
#include "logger/logger.h"
#include "metrics_instance.h"

#if 0
# HELP http_requests_total The total number of HTTP requests.
# TYPE http_requests_total counter
http_requests_total{method="post",code="200"} 1027 1395066363000
http_requests_total{method="post",code="400"}    3 1395066363000
#endif

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

void Metrics::operator()(Session &&session, std::shared_ptr<Chain> chain)
{
    logger()->info("Processing {}", session.req);

    // Replace the send function with 'metricSend' which increments req/response counters.

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
