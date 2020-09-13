#pragma once

#include <vector>
#include <functional>
#include "stalk/stalk_types.h"
#include "logger/logger.h"
#include "middleware/middleware.h"

namespace boost
{
namespace asio
{
class io_context;
}
}

namespace Middleware
{

class Delayer : public std::enable_shared_from_this<Delayer>
{
public:
    Delayer(boost::asio::io_context& ioc);

    void process(Session&& session, std::shared_ptr<Chain> chain);
    void operator()(Session&& session, std::shared_ptr<Chain> chain);

private:

    boost::asio::io_context& ioc;

    std::shared_ptr<Logger> logger;
};

} // namespace Middleware
