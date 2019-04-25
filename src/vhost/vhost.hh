/**
 * \file vhost/vhost.hh
 * \brief VHost declaration.
 */

#pragma once

#include <memory>

#include "config/config.hh"
#include "error/not-implemented.hh"
#include "misc/openssl/ssl.hh"
#include "request/request.hh"
#include "vhost/apm.hh"
#include "vhost/connection.hh"

namespace http
{
    // FIXME: iterator to data remaining from next request.
    using remaining_iterator = int;
    /**
     * \class VHost
     * \brief Abstract class representing a VHost.
     */
    class VHost
    {
    public:
        /**
         * \brief Create a VHost from its configuration.
         *
         * \param conf VHostConfig virtual host configuration.
         */
        explicit VHost(const VHostConfig& conf)
            : conf_(conf)
        {
            if (conf.ssl_cert_ == "")
                return;

            const SSL_METHOD* method;
            method = SSLv23_server_method();
            ssl_ctx_ =
                std::shared_ptr<SSL_CTX>(SSL_CTX_new(method), SSL_CTX_free);

            if (!ssl_ctx_.get())
            {
                perror("Unable to create the SSL Context");
                ERR_print_errors_fp(stderr);
                throw;
            }

            SSL_CTX_set_ecdh_auto(ssl_ctx_, 1);

            if (SSL_CTX_use_PrivateKey_file(
                    ssl_ctx_.get(), conf.ssl_key_.c_str(), SSL_FILETYPE_PEM)
                <= 0)
                throw std::logic_error("invalid SSL privatekey");
            if (SSL_CTX_use_certificate_file(
                    ssl_ctx_.get(), conf.ssl_cert_.c_str(), SSL_FILETYPE_PEM)
                <= 0)
                throw std::logic_error("invalid SSL certificate");

            if (SSL_CTX_check_private_key(ssl_ctx_.get()) != 1)
                throw std::logic_error(
                    "SSL privatekey doesn't match certificate");
        }

        VHost() = delete;
        VHost(const VHost&) = delete;
        VHost& operator=(const VHost&) = delete;
        VHost(VHost&&) = delete;
        VHost& operator=(VHost&&) = delete;
        virtual ~VHost() = default;

        /**
         * \brief Send response depending on the VHost type.
         *
         * \param req Request.
         * \param conn Connection.
         * \param begin remaining_iterator of received data.
         * \param end remaining_iterator of received data.
         */
        virtual void respond(Request&, Connection, remaining_iterator,
                             remaining_iterator) = 0;

        inline const VHostConfig& conf_get() const noexcept
        {
            return conf_;
        }

        inline const std::shared_ptr<SSL_CTX> ssl_ctx_get() const noexcept
        {
            return ssl_ctx_;
        }

        inline void set_ipv6(bool b)
        {
            conf_.is_ipv6_ = b;
        }

        APM& get_apm()
        {
            return apm;
        }

        void apply_set_remove_header(bool is_proxy, std::string& head);

    protected:
        /**
         *  \brief VHost configuration.
         */
        VHostConfig conf_;
        Upstream lastUpstream;//FIXME hoping thomas will answer to me someday
        APM apm;
        /**
         * \brief VHost's SSL context.
         *
         * From ssl(3):
         *
         * SSL_CTX (SSL Context)
         *   This is the global context structure which is created by a server
         *   or client once per program life-time and which holds mainly default
         *   values for the SSL structures which are later created for the
         *   connections.
         *
         * Warning: with this unique_ptr syntax, you'll need to instanciate the
         * pointer with both a value and a Deleter function.
         */

        std::shared_ptr<SSL_CTX> ssl_ctx_;
    };

    using shared_vhost = std::shared_ptr<VHost>;
} // namespace http
