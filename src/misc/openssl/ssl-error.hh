/**
 * \file misc/openssl/ssl-error.hh
 * \brief Implementation of an extension to system_errors for OpenSSL.
 */

#pragma once

#include <openssl/ssl.h>
#include <system_error>

namespace ssl
{
    /**
     * \enum SSL_ERROR
     * \brief Possible errors returned by OpenSSL.
     */
    enum SSL_ERROR
    {
        NONE = SSL_ERROR_NONE,
        ZERO_RETURN = SSL_ERROR_ZERO_RETURN,
        WANT_READ = SSL_ERROR_WANT_READ,
        WANT_WRITE = SSL_ERROR_WANT_WRITE,
        WANT_CONNECT = SSL_ERROR_WANT_CONNECT,
        WANT_ACCEPT = SSL_ERROR_WANT_ACCEPT,
        WANT_X509_LOOKUP = SSL_ERROR_WANT_X509_LOOKUP,
        WANT_ASYNC = SSL_ERROR_WANT_ASYNC,
        WANT_ASYNC_JOB = SSL_ERROR_WANT_ASYNC_JOB,
        WANT_CLIENT_HELLO_CB = SSL_ERROR_WANT_CLIENT_HELLO_CB,
        SYSCALL = SSL_ERROR_SYSCALL,
        SSL = SSL_ERROR_SSL
    };

    inline constexpr const char* ssl_message(SSL_ERROR error)
    {
        switch (error)
        {
        case NONE:
            return "Success.";
        case ZERO_RETURN:
            return "Connection gracefully closed.";
        case WANT_READ:
            return "Not enough data to read.";
        case WANT_WRITE:
            return "Not enough data to write.";
        case WANT_CONNECT:
            return "Should establish connection.";
        case WANT_ACCEPT:
            return "Should accept connection.";
        case WANT_X509_LOOKUP:
            return "Certificate needed.";
        case WANT_ASYNC:
            return "Async needed.";
        case WANT_ASYNC_JOB:
            return "Not async job available";
        case WANT_CLIENT_HELLO_CB:
            return "Waiting for callback to complete.";
        case SYSCALL:
            return "Non-recoverable IO error.";
        case SSL:
            return "SSL failure.";
        default:
            return "Unknown SSL error.";
        }
    }

    class ssl_error_category : public std::error_category
    {
    public:
        virtual const char* name() const noexcept override
        {
            return "ssl_error_category";
        }

        virtual std::string message(int value) const override
        {
            return ssl_message(static_cast<SSL_ERROR>(value));
        }
    };

    const std::error_category& ssl_category();

} // namespace ssl

namespace std
{
    template <>
    struct is_error_code_enum<ssl::SSL_ERROR> : public std::true_type
    {};

    inline std::error_code make_error_code(ssl::SSL_ERROR e);
} // namespace std
