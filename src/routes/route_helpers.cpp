#include "route_helpers.h"
#include "stalk/stalk_server.h"

namespace Routes
{


void addRouteWithChain(std::shared_ptr<Stalk::WebServer> server, const std::string& path, std::set<Stalk::Verb> methods,
                       std::shared_ptr<Middleware::Chain> chain, Middleware::Chain::ChainFn fn)
{
    auto route = Stalk::Route::Http(
        path,
        methods,
        [path, chain, fn](Stalk::ConnectionDetail detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::SendResponse&& send)
            {
                chain->add(fn);

                Middleware::Session session(std::move(detail), std::move(req), std::move(variables), Stalk::Response(), std::move(send));
                chain->process(std::move(session));
            });
    route.setAcceptLongerPaths();
    server->addHttpRoute(std::move(route));
}

} // namespace Routes
