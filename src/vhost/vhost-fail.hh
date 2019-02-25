/**
 * \file vhost/vhost-fail.hh
 * \brief VHostFail declaration.
 */

#pragma once

#include "config/config.hh"
#include "request/request.hh"
#include "vhost/connection.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class VHostFail
     * \brief VHost serving static files.
     */
    class VHostFail : public VHost
    {
    public:
        friend class VHostFactory;
        virtual ~VHostFail() = default;

    private:
        /**
         * \brief Constructor called by the factory.
         *
         * \param config VHostConfig virtual host configuration.
         */
        explicit VHostFail(const VHostConfig& conf)
            : VHost(conf)
        {}

    public:
        /**
         * \brief Send response.
         *
         * \param req Request.
         * \param conn Connection.
         * \param begin remaining_iterator of received data.
         * \param end remaining_iterator of received data.
         *
         * Note that these iterators will only be useful starting from SRPS.
         */
        void respond(const Request&, Connection&, remaining_iterator,
                     remaining_iterator) final;
    };
} // namespace http
