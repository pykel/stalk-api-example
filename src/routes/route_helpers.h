#pragma once

#include <string>
#include <set>
#include "stalk/stalk_verb.h"
#include "middleware/middleware.h"

namespace Stalk
{
class WebServer;
}

namespace Routes
{

/// Helper function that adds handler function (ChainFn) to chain (Chain), and adds chain processing as route to the WebServer.
void addRouteWithChain(std::shared_ptr<Stalk::WebServer> server, const std::string& path, std::set<Stalk::Verb> methods,
                       std::shared_ptr<Middleware::Chain> chain, Middleware::Chain::ChainFn fn);

} // namespace Routes
