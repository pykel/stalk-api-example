#pragma once

#include <memory>

namespace boost { namespace asio {
class io_context;
} }

namespace Stalk
{
class WebServer;
}

class CoreRoutes
{
public:
    CoreRoutes(boost::asio::io_context& ioc, std::shared_ptr<Stalk::WebServer> server);

    boost::asio::io_context& ioc_;
    std::shared_ptr<Stalk::WebServer> server_;
};
