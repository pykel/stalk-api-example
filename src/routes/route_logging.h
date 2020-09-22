#pragma once

#include <memory>

namespace Stalk
{
class WebServer;
}

class RouteLogging
{
public:
    RouteLogging(std::shared_ptr<Stalk::WebServer> server);
};

