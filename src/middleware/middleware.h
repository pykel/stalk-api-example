#pragma once

#include <vector>
#include <functional>
#include "stalk/stalk_types.h"

namespace Middleware
{

struct Session
{
    Session() = default;
    Session(Stalk::ConnectionDetail&& detail, Stalk::Request&& req, Stalk::RequestVariables&& variables, Stalk::Response&& resp, Stalk::SendResponse&& send);

    Stalk::ConnectionDetail detail;
    Stalk::Request req;
    Stalk::RequestVariables variables;
    Stalk::Response resp;
    Stalk::SendResponse send;
};

class Chain : public std::enable_shared_from_this<Chain>
{
public:

    using ChainFn = std::function<void(Session&& session, std::shared_ptr<Chain> chain)>;
    using Container = std::vector<ChainFn>;

    Chain() = default;
    ~Chain() = default;
    Chain(const Container& chain);
    Chain(Container&& chain);

    Chain& add(ChainFn fn);

    ChainFn next();

    void process(Session&& session);

private:
    Container chain_;
    Container::iterator next_;
};

} // namespace Middleware
