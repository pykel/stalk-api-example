#pragma once

#include "middleware/middleware.h"

namespace Middleware
{

class Metrics
{
public:
    void operator()(Session&& session, std::shared_ptr<Chain> chain);
};

}
