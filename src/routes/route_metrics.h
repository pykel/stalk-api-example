#pragma once

#include <memory>

namespace Stalk
{
class WebServer;
}

class RouteMetrics
{
public:
    RouteMetrics(std::shared_ptr<Stalk::WebServer> server);
};
