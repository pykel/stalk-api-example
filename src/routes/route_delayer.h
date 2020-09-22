#pragma once

#include <memory>


namespace boost { namespace asio {
class io_context;
} }

namespace Stalk
{
class WebServer;
}

class RouteDelayer
{
public:
    RouteDelayer(boost::asio::io_context& ioc, std::shared_ptr<Stalk::WebServer> server);
};

