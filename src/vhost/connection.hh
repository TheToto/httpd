/**
 * \file vhost/connection.hh
 * \brief Connection declaration.
 */

#pragma once

#include <memory>

#include "socket/socket.hh"

namespace http
{
    class VHost;
    /**
     * \struct Connection
     * \brief Value object representing a connection.
     *
     * We need to keep track of the state of each request while it has not
     * been fully processed.
     */
    struct Connection
    {
        Connection(shared_socket sock, std::shared_ptr<VHost> vhost, int health)
                : backend_(sock)
                , vhost_(vhost)
                , health_(health)
        {}

        Connection(shared_socket sock, std::shared_ptr<VHost> vhost, Request& req)
                : sock_(sock)
                , vhost_(vhost)
                , req_(req)
        {
            health_ = -1;
        }

        Connection() = default;
        Connection(const Connection&) = default;
        Connection& operator=(const Connection&) = default;
        Connection(Connection&&) = default;
        Connection& operator=(Connection&&) = default;
        ~Connection() = default;

        bool is_health()
        {
            return health_ != -1;
        }

        shared_socket sock_;
        shared_socket backend_ = nullptr;
        std::shared_ptr<VHost> vhost_;
        Request req_;
        int health_;
        /* FIXME: Add members to store the information relative to the
        ** connection.
        */
    };
} // namespace http
