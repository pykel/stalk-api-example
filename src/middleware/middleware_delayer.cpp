#include "middleware_delayer.h"
#include <boost/asio.hpp>
#include "logger/logger.h"

namespace Middleware
{


Delayer::Delayer(boost::asio::io_context& ioc) : ioc(ioc)
{
    logger = Logger::get("Middleware.Delayer");
}

void Delayer::process(Session&& session, std::shared_ptr<Chain> chain)
{
    logger->info("Processing {}", session.req);

    session.req.set("TestHeader", "8.8");

    auto timer = std::make_shared<boost::asio::steady_timer>(ioc);

    auto self = shared_from_this();

    // respond after timer fires
    timer->expires_after(std::chrono::seconds(1));
    timer->async_wait([self, this, timer, session{std::move(session)}, chain]
                      (const boost::system::error_code&) mutable
        {
            logger->info("Async Delay Completed {}", session.req);

            session.resp.status(Stalk::Status::ok)
                .set("test-header", "test-header-value");

            // Move to the next middleware
            chain->process(std::move(session));
        });
}

void Delayer::operator()(Session&& session, std::shared_ptr<Chain> chain)
{
    process(std::move(session), chain);
}

} // namespace Middleware
