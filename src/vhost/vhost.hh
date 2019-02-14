/**
 * \file vhost/vhost.hh
 * \brief VHost declaration.
 */

#pragma once

#include "config/config.hh"
#include "error/not-implemented.hh"
#include "request/request.hh"
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
        {}

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
        virtual void respond(const Request&, Connection, remaining_iterator,
                             remaining_iterator) = 0;

        inline const VHostConfig& conf_get() const noexcept
        {
            return conf_;
        }

    private:
        /**
         *  \brief VHost configuration.
         */
        VHostConfig conf_;
    };

    using shared_vhost = std::shared_ptr<VHost>;
} // namespace http
