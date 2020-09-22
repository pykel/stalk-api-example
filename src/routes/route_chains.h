#pragma once

#include "middleware/middleware.h"
#include "middleware/middleware_metrics.h"
#include "middleware/middleware_secure_header_hoister.h"

namespace Routes
{

inline std::shared_ptr<Middleware::Chain> buildRouteChainBasic(const std::string& path)
{
    // Create middleware chain
    auto chain = std::make_shared<Middleware::Chain>(path);

    chain->add(Middleware::Metrics());
    chain->add(Middleware::SecureHeaderHoister());

    return chain;
}
#if 0
inline routeAddWithChain(chain, path, methods, ChainFn fn)
{

}
#endif
} // namespace Routes
