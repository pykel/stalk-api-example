#include "middleware_secure_header_hoister.h"
#include "logger/logger.h"
#include "utils/string_transform.h"


namespace {

std::shared_ptr<Logger> logger()
{
    static std::shared_ptr<Logger> l = Logger::get("Middleware.SecureHeaderHoister");
    return l;
}

} // anon namespace

namespace Middleware
{

namespace SecureHeaders
{

    constexpr const char* headerName() { return "x-forwarded-client-cert"; }
    const char* headerString(Header header)
    {
        switch (header)
        {
            case Header::By:
                return "By";
            case Header::Hash:
                return "Hash";
            case Header::Cert:
                return "Cert";
            case Header::Chain:
                return "Chain";
            case Header::Subject:
                return "Subject";
            case Header::URI:
                return "URI";
            case Header::DNS:
                return "DNS";
            case Header::Unknown:
                return "Unknown";
        }

        return "Unknown";
    }

    Header headerFromString(const std::string& str)
    {
        static const std::map<std::string, Header> headerMap = {
            { "By", Header::By },
            { "Hash", Header::Hash },
            { "Cert", Header::Cert },
            { "Chain", Header::Chain },
            { "Subject", Header::Subject },
            { "URI", Header::URI },
            { "DNS", Header::DNS }
        };

        auto it = headerMap.find(str);
        return it == headerMap.end() ? Header::Unknown : it->second;
    }

    static const std::string separator = ";";
}

SecureHeaderHoister::SecureHeaderHoister(const std::set<SecureHeaders::Header>& headers) : headers_(headers)
{
}

SecureHeaderHoister::SecureHeaderHoister(std::set<SecureHeaders::Header>&& headers) : headers_(std::move(headers))
{
}


void SecureHeaderHoister::operator()(Session&& session, std::shared_ptr<Chain> chain)
{
    logger()->info("Processing {}", session.req);

    static const auto append = [](std::string& dest, const std::string& key, const std::string& value)
        {
            if (!dest.empty())
                dest += SecureHeaders::separator;

            dest += value.empty() ? key : key + "=" + value;
        };

    static const auto appendEncodedIfNotEmpty = [](std::string& dest, const std::string& key, const std::string& value)
        {
            if (!value.empty())
            {
                const auto encoded = Utils::String::replace_all(value,
                                        {{"\n", "%0A"}, {" ", "%20"}, {"+", "%2B"}, {"/", "%2F"}, {"=", "%3D"}});
                append(dest, key, encoded);
            }
        };

    if (session.detail.encrypted)
    {
        std::string value;
        const auto& security = session.detail.security;

        for (const auto& hdr : headers_)
        {
            switch (hdr)
            {
                case SecureHeaders::Header::Hash:
                {
                    appendEncodedIfNotEmpty(value, SecureHeaders::headerString(SecureHeaders::Header::Hash), security.peerCert.digest);
                    break;
                }
                case SecureHeaders::Header::Cert:
                {
                    appendEncodedIfNotEmpty(value, SecureHeaders::headerString(SecureHeaders::Header::Cert), security.peerCert.pem);
                    break;
                }
                case SecureHeaders::Header::Subject:
                {
                    appendEncodedIfNotEmpty(value, SecureHeaders::headerString(SecureHeaders::Header::Subject), security.peerCert.commonName);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        if (!value.empty())
        {
            session.req.set("x-forwarded-client-cert", value);
        }
    }

    // Move to the next middleware
    chain->process(std::move(session));
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
