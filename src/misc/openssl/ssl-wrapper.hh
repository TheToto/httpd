/**
 * \file misc/openssl/ssl-wrapper.hh
 * \brief SSLWrapper declaration.
 */

#pragma once

#include <functional>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <utility>

#include "error/parsing-error.hh"

namespace ssl
{
    /**
     * \class SSLWrapper
     * \brief Wrapping around OpenSSL functions to convert failures into
     * exceptions.
     *
     * Wrapping OpenSSL functions this way helps to ensure the RAII idiom.
     */
    template <typename RetType, typename FuncRetType, typename... Args>
    class SSLWrapper
    {
    public:
        SSLWrapper(FuncRetType func(Args...))
            : func_{func}
        {}

        template <typename... UniversalArgs>
        RetType operator()(SSL* ssl, UniversalArgs&&... args)
        {
            FuncRetType ret;

            do
            {
                errno = 0;
                ret = func_(ssl, std::forward<UniversalArgs>(args)...);
            } while (errno == EINTR);

            if (ret <= FuncRetType(0) && errno != EWOULDBLOCK
                && errno != EAGAIN)
            {
                throw std::system_error(SSL_get_error(ssl, ret),
                                        std::system_category());
            }

            return RetType(ret);
        }

    private:
        std::function<FuncRetType(Args...)> func_;
    };

    /**
     * \brief Helper function to build SSLWrapper.
     */
    template <typename RetType, typename FuncRetType, typename... Args>
    SSLWrapper<RetType, FuncRetType, Args...>
    make_ssl_wrapper(FuncRetType func(Args...))
    {
        return SSLWrapper<RetType, FuncRetType, Args...>(func);
    }

    /**
     * \class SSLCtxWrapper
     * \brief Wrapping around OpenSSL CTX functions to convert failures into
     * exceptions.
     *
     * Wrapping OpenSSL CTX functions this way helps to ensure the RAII idiom.
     */
    template <typename RetType, typename FuncRetType, typename... Args>
    class SSLCtxWrapper
    {
    public:
        SSLCtxWrapper(FuncRetType func(Args...))
            : func_{func}
        {}

        template <typename... UniversalArgs>
        RetType operator()(const std::string& error_string,
                           UniversalArgs&&... args)
        {
            auto ret = func_(std::forward<UniversalArgs>(args)...);

            if (ret <= FuncRetType(0))
            {
                throw http::ParsingError(
                    error_string + ": "
                    + ERR_error_string(ERR_get_error(), nullptr));
            }

            return RetType(ret);
        }

    private:
        std::function<FuncRetType(Args...)> func_;
    };

    /**
     * \brief Helper function to build SSLCtxWrapper.
     */
    template <typename RetType, typename FuncRetType, typename... Args>
    SSLCtxWrapper<RetType, FuncRetType, Args...>
    make_ssl_ctx_wrapper(FuncRetType func(Args...))
    {
        return SSLCtxWrapper<RetType, FuncRetType, Args...>(func);
    }
} // namespace ssl
