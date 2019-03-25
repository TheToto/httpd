/**
 * \file events/client.hh
 * \brief ClientEW declaration.
 */

#pragma once

#include <exception>
#include <iostream>

#include "events/events.hh"
#include "events/register.hh"
#include "misc/socket.hh"
#include "request/error.hh"
#include "request/request.hh"
#include "socket/socket.hh"
#include "vhost/connection.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class ClientProxyEW
     * \brief Workflow for client socket.
     */
    class ClientProxyEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a ClientProxyEW from a Client socket.
         */
        explicit ClientProxyEW(Connection conn)
            : EventWatcher(conn.backend_->fd_get()->fd_, EV_READ)
        {
            sock_ = conn.backend_;
            // Set socket non block
            int tmpfd = sock_->fd_get()->fd_;
            int flags = fcntl(tmpfd, F_GETFL);
            flags |= O_NONBLOCK;
            fcntl(tmpfd, F_SETFL, flags);
        }

        /**
         * \brief Start accepting connections on Client socket.
         */
        void operator()() final
        {
            char str_c[8192];
            int n = 0;
            try
            {
                n = sock_->recv(str_c, 8192);
                if (n <= 0)
                {
                    std::clog << "The backend has disconnect\n";
                    event_register.unregister_ew(this);
                    // SEND BAD GATEWAY
                    return;
                }
            }
            catch (const std::exception& e)
            {
                std::clog << "The backend has disconnect\n";
                event_register.unregister_ew(this);
                // SEND BAD GATEWAY
                return;
            }

            bool is_complete = false; // FIXME : Test if \n\r\n\r
            conn_.vhost_->apply_set_remove_header(false, header_);
            if (is_complete)
            {
                std::clog << "We have the backend response ! \n" << std::endl;
                event_register.unregister_ew(this);
                Response r(header_);
                r.file_ = conn_.sock_->fd_get();
                r.file_size_ =
                    /* FIXME : Compute file size from content lenght */ 0;
                event_register.register_ew<SendResponseEW>(conn_.sock_, r);
            }
            else
            {
                std::clog
                    << "Current response from backend is not complete...\n";
            }
        }

    private:
        /**
         * \brief Client socket.
         */
        Connection conn_;
        shared_socket sock_;
        std::string header_;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
