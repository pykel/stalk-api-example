#pragma once

#include <set>
#include <functional>
#include "middleware.h"

namespace Middleware
{

namespace SecureHeaders
{
    enum class Header
    {
        By,
        Hash,
        Cert,
        Chain,
        Subject,
        URI,
        DNS,
        Unknown
    };

    constexpr const char* headerName();
    const char* headerString(Header header);
    Header headerFromString(const char* str);
}

class SecureHeaderHoister
{
public:

    SecureHeaderHoister(const std::set<SecureHeaders::Header>& headers);
    SecureHeaderHoister(std::set<SecureHeaders::Header>&& headers);

    void operator()(Session&& session, std::shared_ptr<Chain> chain);

private:
    std::set<SecureHeaders::Header> headers_;
};

} // namespace Middleware
