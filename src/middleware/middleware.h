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
    /// The signature of routed calls through the middleware.
    using ChainFn = std::function<void(Session&& session, std::shared_ptr<Chain> chain)>;
    using Container = std::vector<ChainFn>;

    Chain(const std::string& key = std::string()) : key_(key) {}
    ~Chain() = default;
    Chain(const Container& chain);
    Chain(Container&& chain);

    Chain& add(ChainFn fn);
#if 0
    template<typename T>
    Chain& add(const T& functor)
    {
        chain_.push_back([functor](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> chain) mutable
            {
                functor(std::move(session), chain);
            });

        next_ = chain_.begin();
        return *this;
    }

    template<typename T>
    Chain& add(T&& functor)
    {
        chain_.push_back([functor = std::move(functor)](Middleware::Session&& session, std::shared_ptr<Middleware::Chain> chain)
            {
                functor(std::move(session), chain);
            });

        next_ = chain_.begin();
        return *this;
    }
#endif
    ChainFn next();

    void process(Session&& session);

    std::string key() const { return key_; }

private:
    std::string key_;
    Container chain_;
    Container::iterator next_;
};

template<typename T>
inline std::shared_ptr<Chain> chainBuilder(std::shared_ptr<Chain> chain, T v)
{
    chain->add(v);
    return chain;
}

template<typename T, typename... Args>
inline std::shared_ptr<Chain> chainBuilder(std::shared_ptr<Chain> chain, T first, Args... args)
{
    chain->add(first);
    return chainBuilder(chain, args...);
}

} // namespace Middleware
