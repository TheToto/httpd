#include "misc/openssl/ssl-error.hh"

namespace ssl
{
    const std::error_category& ssl_category()
    {
        static ssl_error_category instance;
        return instance;
    }

} // namespace ssl

inline std::error_code std::make_error_code(ssl::SSL_ERROR e)
{
    return std::error_code(static_cast<int>(e), ssl::ssl_category());
}
