#pragma once

#include <vector>
#include <functional>
#include "middleware.h"
#include "stalk/stalk_types.h"
#include "logger/logger.h"

namespace Middleware
{

class SecureHeaderHoister
{
public:
    SecureHeaderHoister();

    void process(Session&& session, std::shared_ptr<Chain> chain);
    void operator()(Session&& session, std::shared_ptr<Chain> chain);

private:
    Logger::Log logger;
};

} // namespace Middleware
