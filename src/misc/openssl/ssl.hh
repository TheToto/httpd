/**
 * \file misc/openssl/ssl.h
 * \brief OpenSSL wrappers.
 */

#pragma once

#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#include "misc/openssl/ssl-wrapper.hh"

namespace ssl
{
    /**
     * \brief SSL_accept(3).
     */
    inline auto accept = make_ssl_wrapper<void>(::SSL_accept);

    /**
     * \brief SSL_connect(3).
     */
    inline auto connect = make_ssl_wrapper<void>(::SSL_connect);

    /**
     * \brief SSL_CTX_check_private_key(3).
     */
    inline auto ctx_check_private_key =
        make_ssl_ctx_wrapper<void>(::SSL_CTX_check_private_key);

    /**
     * \brief SSL_CTX_use_certificate_file(3).
     */
    inline auto ctx_use_certificate_file =
        make_ssl_ctx_wrapper<void>(::SSL_CTX_use_certificate_file);

    /**
     * \brief SSL_CTX_use_PrivateKey_file(3).
     */
    inline auto ctx_use_PrivateKey_file =
        make_ssl_ctx_wrapper<void>(::SSL_CTX_use_PrivateKey_file);

    /**
     * \brief SSL_read(3).
     */
    inline auto read = make_ssl_wrapper<int>(::SSL_read);

    /**
     * \brief SSL_write(3).
     */
    inline auto write = make_ssl_wrapper<int>(::SSL_write);

    /**
     * \brief X509_check_host(3).
     */
    inline auto x509_check_host = make_ssl_ctx_wrapper<void>(::X509_check_host);
} // namespace ssl
