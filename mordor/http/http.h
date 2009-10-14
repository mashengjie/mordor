#ifndef __MORDOR_HTTP_H__
#define __MORDOR_HTTP_H__
// Copyright (c) 2009 - Decho Corp.

#include <string.h>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include <boost/variant.hpp>
#include <boost/date_time.hpp>

#include "mordor/predef.h"
#include "mordor/string.h"
#include "mordor/uri.h"
#include "mordor/version.h"

namespace Mordor {
namespace HTTP {

struct Exception : virtual Mordor::Exception {};
struct IncompleteMessageHeaderException : virtual Exception, virtual StreamException {};

// Unparseable
struct BadMessageHeaderException : virtual Exception, StreamException {};

struct PriorRequestFailedException : virtual Exception {};

struct ConnectionVoluntarilyClosedException : virtual PriorRequestFailedException {};

// Logically doesn't make sense
struct InvalidMessageHeaderException : virtual Exception, virtual StreamException {
public:
    InvalidMessageHeaderException() {}
    InvalidMessageHeaderException(const std::string &message)
        : m_message(message) {}
    ~InvalidMessageHeaderException() throw() {}

    const char *what() const throw() { return m_message.c_str(); }
private:
    std::string m_message;
};

struct InvalidTransferEncodingException : virtual InvalidMessageHeaderException
{
public:
    InvalidTransferEncodingException(const std::string &message)
        : InvalidMessageHeaderException(message)
    {}
};

enum Method
{
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    CONNECT,
    OPTIONS,
    TRACE
};
extern const char *methods[];

enum Status
{
    INVALID                          = 0,

    CONTINUE                         = 100,
    SWITCHING_PROTOCOL               = 101,
    
    OK                               = 200,
    CREATED                          = 201,
    ACCEPTED                         = 202,
    NON_AUTHORITATIVE_INFORMATION    = 203,
    NO_CONTENT                       = 204,
    RESET_CONTENT                    = 205,
    PARTIAL_CONTENT                  = 206,

    MULTIPLE_CHOICES                 = 300,
    MOVED_PERMANENTLY                = 301,
    FOUND                            = 302,
    SEE_OTHER                        = 303,
    NOT_MODIFIED                     = 304,
    USE_PROXY                        = 305,
    // UNUSED                        = 306,
    TEMPORARY_REDIRECT               = 307,

    BAD_REQUEST                      = 400,
    UNAUTHORIZED                     = 401,
    PAYMENT_REQUIRED                 = 402,
    FORBIDDEN                        = 403,
    NOT_FOUND                        = 404,
    METHOD_NOT_ALLOWED               = 405,
    NOT_ACCEPTABLE                   = 406,
    PROXY_AUTHENTICATION_REQUIRED    = 407,
    REQUEST_TIMEOUT                  = 408,
    CONFLICT                         = 409,
    GONE                             = 410,
    LENGTH_REQUIRED                  = 411,
    PRECONDITION_FAILED              = 412,
    REQUEST_ENTITY_TOO_LARGE         = 413,
    REQUEST_URI_TOO_LONG             = 414,
    UNSUPPORTED_MEDIA_TYPE           = 415,
    REQUESTED_RANGE_NOT_SATISFIABLE  = 416,
    EXPECTATION_FAILED               = 417,

    INTERNAL_SERVER_ERROR            = 500,
    NOT_IMPLEMENTED                  = 501,
    BAD_GATEWAY                      = 502,
    SERVICE_UNAVAILABLE              = 503,
    GATEWAY_TIMEOUT                  = 504,
    HTTP_VERSION_NOT_SUPPORTED       = 505
};
const char *reason(Status s);

class Redirect : public HTTP::Exception
{
public:
    Redirect(Status status, const URI &uri)
        : m_status(status), m_uri(uri)
    {}

    Status status() const { return m_status; }
    const URI &uri() const { return m_uri; }

private:
    Status m_status;
    URI m_uri;
};

std::string quote(const std::string &str, bool alwaysQuote = false, bool comment = false);
std::string unquote(const char *str, size_t size);
std::string unquote(const std::string &str);

struct Version
{
    Version() : major(~0), minor(~0) {}
    Version(unsigned char m, unsigned char n) : major(m), minor(n) {}

    unsigned char major;
    unsigned char minor;

    bool operator==(const Version& rhs) const
    {
        return major == rhs.major && minor == rhs.minor;
    }
    bool operator!=(const Version& rhs) const
    {
        return !(*this == rhs);
    }
    bool operator<(const Version& rhs) const
    {
        if (major < rhs.major) return true; return minor < rhs.minor;
    }
    bool operator<=(const Version& rhs) const
    {
        if (major > rhs.major) return false;
        if (major < rhs.major) return true; return minor <= rhs.minor;
    }
    bool operator>(const Version& rhs) const
    {
        if (major > rhs.major) return true; return minor > rhs.minor;
    }
    bool operator>=(const Version& rhs) const
    {
        if (major < rhs.major) return false;
        if (major > rhs.major) return true; return minor >= rhs.minor;
    }
};

struct ETag
{
    ETag()
        : weak(false), unspecified(true)
    {}
    ETag(const std::string &v, bool w = false)
        : weak(w), unspecified(false), value(v)
    {}

    bool weak, unspecified;
    std::string value;

    bool operator< (const ETag &rhs) const
    {
        if (unspecified && !rhs.unspecified)
            return true;
        if (!unspecified && rhs.unspecified)
            return false;
        if (weak && !rhs.weak)
            return false;
        if (!weak && rhs.weak)
            return true;
        return value < rhs.value;
    }
};

typedef std::set<ETag> ETagSet;

struct Product
{
    std::string product;
    std::string version;
};

typedef std::vector<Product> ProductList;
typedef std::vector<boost::variant<Product, std::string> > ProductAndCommentList;

typedef std::set<std::string, caseinsensitiveless> StringSet;
typedef std::map<std::string, std::string, caseinsensitiveless> StringMap;

struct ValueWithParameters
{
    ValueWithParameters()
    {}
    ValueWithParameters(const char *val) : value(val)
    {}
    ValueWithParameters(const std::string &val) : value(val)
    {}

    std::string value;
    StringMap parameters;
};

typedef std::vector<ValueWithParameters> ParameterizedList;

struct AuthParams
{
    std::string scheme;
    std::string base64;
    StringMap parameters;
};

typedef std::vector<AuthParams> ChallengeList;

struct KeyValueWithParameters
{
    std::string key;
    std::string value;
    StringMap parameters;
};

typedef std::vector<KeyValueWithParameters> ParameterizedKeyValueList;

struct MediaType
{
    std::string type;
    std::string subtype;
    StringMap parameters;
};

typedef std::vector<std::pair<unsigned long long, unsigned long long> > RangeSet;
struct ContentRange
{
    ContentRange() : first(~0ULL), last(~0ULL), instance(~0ULL) {}

    unsigned long long first;
    unsigned long long last;
    unsigned long long instance;
};

struct AcceptValue
{
    AcceptValue() : qvalue(~0u) {}
    AcceptValue(const char *v, unsigned int q = ~0u)
        : value(v), qvalue(q)
    {}
    AcceptValue(const std::string &v, unsigned int q = ~0u)
        : value(v), qvalue(q)
    {}

    std::string value;
    unsigned int qvalue;
};

typedef std::vector<AcceptValue> AcceptList;

struct AcceptValueWithParameters
{
    AcceptValueWithParameters() : qvalue(~0u) {}
    AcceptValueWithParameters(const char *v, unsigned int q = ~0u)
        : value(v), qvalue(q)
    {}
    AcceptValueWithParameters(const std::string &v, unsigned int q = ~0u)
        : value(v), qvalue(q)
    {}

    std::string value;
    StringMap parameters;
    unsigned int qvalue;
    StringMap acceptParams;

    bool operator== (const AcceptValueWithParameters &rhs) const;
    bool operator!= (const AcceptValueWithParameters &rhs) const
    { return !(*this == rhs); }
};

typedef std::vector<AcceptValueWithParameters> AcceptListWithParameters;

struct RequestLine
{
    RequestLine() : method(GET) {}

    Method method;
    URI uri;
    Version ver;        
};

struct StatusLine
{
    StatusLine() : status(OK) {}

    Status status;
    std::string reason;
    Version ver;
};

struct GeneralHeaders
{
    StringSet connection;
    boost::posix_time::ptime date;
    StringSet proxyConnection; // NON-STANDARD!!!!
    ParameterizedList transferEncoding;
    StringSet trailer;
    ProductList upgrade;
};

struct RequestHeaders
{
    AcceptList acceptCharset;
    AcceptList acceptEncoding;
    AuthParams authorization;
    ParameterizedKeyValueList expect;
    std::string host;
    ETagSet ifMatch;
    boost::posix_time::ptime ifModifiedSince;
    ETagSet ifNoneMatch;
    boost::variant<ETag, boost::posix_time::ptime> ifRange;
    boost::posix_time::ptime ifUnmodifiedSince;
    AuthParams proxyAuthorization;
    RangeSet range;
    URI referer;
    AcceptListWithParameters te;
    ProductAndCommentList userAgent;
};

struct ResponseHeaders
{
    StringSet acceptRanges;
    ETag eTag;
    URI location;
    ChallengeList proxyAuthenticate;
    boost::variant<boost::posix_time::ptime, unsigned long long> retryAfter;
    ProductAndCommentList server;
    ChallengeList wwwAuthenticate;
};

struct EntityHeaders
{
    EntityHeaders() : contentLength(~0ull) {}

    std::vector<std::string> contentEncoding;
    unsigned long long contentLength;
    ContentRange contentRange;
    MediaType contentType;
    boost::posix_time::ptime expires;
    boost::posix_time::ptime lastModified;
    StringMap extension;
};

struct Request
{
    RequestLine requestLine;
    GeneralHeaders general;
    RequestHeaders request;
    EntityHeaders entity;

    std::string toString() const;
};

struct Response
{
    StatusLine status;
    GeneralHeaders general;
    ResponseHeaders response;
    EntityHeaders entity;

    std::string toString() const;
};

bool isAcceptable(const ChallengeList &list, const std::string &scheme);
bool isAcceptable(const AcceptListWithParameters &list, const AcceptValueWithParameters &value, bool defaultMissing = false);
bool isPreferred(const AcceptListWithParameters &list, const AcceptValueWithParameters &lhs, const AcceptValueWithParameters &rhs);
const AcceptValueWithParameters *preferred(const AcceptListWithParameters &accept, const AcceptListWithParameters &available);

// Logically the entire response is unexpected
struct InvalidResponseException : virtual HTTP::Exception
{
public:
    InvalidResponseException(const std::string &message, const Response &response)
        : m_message(message),
          m_response(response)
    {}
    ~InvalidResponseException() throw() {}

    const char *what() const throw() { return m_message.c_str(); }
    const Response &response() { return m_response; }

private:
    std::string m_message;
    Response m_response;
};

std::ostream& operator<<(std::ostream& os, Method m);
std::ostream& operator<<(std::ostream& os, Status s);
std::ostream& operator<<(std::ostream& os, Version v);
std::ostream& operator<<(std::ostream& os, const ETag &e);
std::ostream& operator<<(std::ostream& os, const ETagSet &v);
std::ostream& operator<<(std::ostream& os, const Product &p);
std::ostream& operator<<(std::ostream& os, const ProductList &l);
std::ostream& operator<<(std::ostream& os, const ProductAndCommentList &l);
std::ostream& operator<<(std::ostream& os, const ValueWithParameters &v);
std::ostream& operator<<(std::ostream& os, const ParameterizedList &l);
std::ostream& operator<<(std::ostream& os, const AuthParams &v);
std::ostream& operator<<(std::ostream& os, const ChallengeList &l);
std::ostream& operator<<(std::ostream& os, const KeyValueWithParameters &v);
std::ostream& operator<<(std::ostream& os, const ParameterizedKeyValueList &v);
std::ostream& operator<<(std::ostream& os, const MediaType &m);
std::ostream& operator<<(std::ostream& os, const ContentRange &m);
std::ostream& operator<<(std::ostream& os, const AcceptValue &v);
std::ostream& operator<<(std::ostream& os, const AcceptList &l);
std::ostream& operator<<(std::ostream& os, const AcceptValueWithParameters &v);
std::ostream& operator<<(std::ostream& os, const AcceptListWithParameters &l);
std::ostream& operator<<(std::ostream& os, const RequestLine &r);
std::ostream& operator<<(std::ostream& os, const StatusLine &s);
std::ostream& operator<<(std::ostream& os, const GeneralHeaders &g);
std::ostream& operator<<(std::ostream& os, const RequestHeaders &r);
std::ostream& operator<<(std::ostream& os, const ResponseHeaders &r);
std::ostream& operator<<(std::ostream& os, const EntityHeaders &e);
std::ostream& operator<<(std::ostream& os, const Request &r);
std::ostream& operator<<(std::ostream& os, const Response &r);

}}

#endif