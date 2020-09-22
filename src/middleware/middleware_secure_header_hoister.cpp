#include "middleware_secure_header_hoister.h"
#include "logger/logger.h"

namespace Middleware
{


namespace Headers
{

struct XFCC
{
    static const std::string key;
    static const std::string separator;

    static const std::string valueBy;
    static const std::string valueHash;
    static const std::string valueCert;
    static const std::string valueChain;
    static const std::string valueSubject;
    static const std::string valueURI;
    static const std::string valueDNS;
};

const std::string XFCC::key = "x-forwarded-client-cert";
const std::string XFCC::separator = ";";
const std::string XFCC::valueBy = "By";
const std::string XFCC::valueHash = "Hash";
const std::string XFCC::valueCert = "Cert";
const std::string XFCC::valueChain = "Chain";
const std::string XFCC::valueSubject = "Subject";
const std::string XFCC::valueURI = "URI";
const std::string XFCC::valueDNS = "DNS";

} // namespace Headers

SecureHeaderHoister::SecureHeaderHoister() :
    logger(Logger::get("Middleware.SecureHeaderHoister"))
{
}

void SecureHeaderHoister::process(Session&& session, std::shared_ptr<Chain> chain)
{
    session.req.set(Stalk::Field::version, "8.8");

    auto appendIfNotEmpty = [](std::string& dest, const std::string& key, const std::string& value)
        {
            if (!value.empty())
            {
                if (!dest.empty())
                    dest += Headers::XFCC::separator;

                dest += value.empty() ? key : key + "=" + value;
            }
        };

    auto build = [&](const Stalk::ConnectionDetail::Security& security) -> std::string
        {
            std::string value;
            appendIfNotEmpty(value, Headers::XFCC::valueHash, security.peerCert.digest);
            return value;
        };

    if (session.detail.encrypted && !session.detail.security.peerCert.pem.empty())
    {
        session.req.set("x-forwarded-client-cert", build(session.detail.security));
    }

    logger->info("process: Hoisting headers : {}", session.req);

    // Move to the next middleware
    chain->process(std::move(session));
}

void SecureHeaderHoister::operator()(Session&& session, std::shared_ptr<Chain> chain)
{
    process(std::move(session), chain);
}

} // namespace Middleware

#if 0

Envoy proxy header hoisting reference
x-forwarded-client-cert XFCC

x-forwarded-client-cert

x-forwarded-client-cert (XFCC) is a proxy header which indicates certificate information of part or all of the clients or proxies that a request has flowed through, on its way from the client to the server. A proxy may choose to sanitize/append/forward the XFCC header before proxying the request.

The XFCC header value is a comma (",") separated string. Each substring is an XFCC element, which holds information added by a single proxy. A proxy can append the current client certificate information as an XFCC element, to the end of the requests XFCC header after a comma.

Each XFCC element is a semicolon ";" separated string. Each substring is a key-value pair, grouped together by an equals ("=") sign. The keys are case-insensitive, the values are case-sensitive. If ",", ";" or "=" appear in a value, the value should be double-quoted. Double-quotes in the value should be replaced by backslash-double-quote (")  ".

The following keys are supported:

By The Subject Alternative Name (URI type) of the current proxys certificate.
Hash The SHA 256 digest of the current client certificate.
Cert The entire client certificate in URL encoded PEM format.
Chain The entire client certificate chain (including the leaf certificate) in URL encoded PEM format.
Subject The Subject field of the current client certificate. The value is always double-quoted.
URI The URI type Subject Alternative Name field of the current client certificate.
DNS The DNS type Subject Alternative Name field of the current client certificate. A client certificate may contain multiple DNS type Subject Alternative Names, each will be a separate key-value pair.


For one client certificate with only URI type Subject Alternative Name:
x-forwarded-client-cert: By=http://frontend.lyft.com;Hash=468ed33be74eee6556d90c0149c1309e9ba61d6425303443c0748a02dd8de688;Subject="/C=US/ST=CA/L=San Francisco/OU=Lyft/CN=Test Client";URI=http://testclient.lyft.com

For two client certificates with only URI type Subject Alternative Name:
x-forwarded-client-cert: By=http://frontend.lyft.com;Hash=468ed33be74eee6556d90c0149c1309e9ba61d6425303443c0748a02dd8de688;URI=http://testclient.lyft.com,By=http://backend.lyft.com;Hash=9ba61d6425303443c0748a02dd8de688468ed33be74eee6556d90c0149c1309e;URI=http://frontend.lyft.com

For one client certificate with both URI type and DNS type Subject Alternative Name:
x-forwarded-client-cert: By=http://frontend.lyft.com;Hash=468ed33be74eee6556d90c0149c1309e9ba61d6425303443c0748a02dd8de688;Subject="/C=US/ST=CA/L=San Francisco/OU=Lyft/CN=Test Client";URI=http://testclient.lyft.com;DNS=lyft.com;DNS=www.lyft.com


#endif
