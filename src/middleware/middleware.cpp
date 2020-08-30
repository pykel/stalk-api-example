#include "middleware.h"
#include "logger/logger.h"

namespace Middleware
{

Session::Session(Stalk::ConnectionDetail&& detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::Response&& resp, Stalk::SendResponse&& send) :
    detail{std::move(detail)}, req{std::move(req)}, variables{std::move(variables)}, resp{std::move(resp)}, send{std::move(send)}
{
}


Chain::Chain(const Container& chain) : chain_(chain), next_(chain_.begin())  {}
Chain::Chain(Container&& chain) : chain_(std::move(chain)), next_(chain_.begin()) {}

Chain& Chain::add(ChainFn fn) { chain_.push_back(fn); next_ = chain_.begin(); return *this; }

Chain::ChainFn Chain::next()
{
    if (next_ == chain_.end())
        return ChainFn();

    auto it = next_++;

    return *it;
}

void Chain::process(Session&& session)
{
    ChainFn chainFn = next();
    if (chainFn)
    {
        auto self = shared_from_this();
        chainFn(std::move(session), self);
    }
    else
    {
        session.send(std::move(session.resp));
    }
}

} // namespace Middleware
