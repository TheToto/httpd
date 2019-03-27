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
#include "vhost/apm.hh"
#include "vhost/connection.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost.hh"
namespace http
{
    /**
     * \class ClientEW
     * \brief Workflow for client socket.
     */
    class ClientEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a ClientEW from a Client socket.
         */
        explicit ClientEW(shared_socket socket)
            : EventWatcher(socket->fd_get()->fd_, EV_READ)
        {
            APM::global_connections_reading++;
            sock_ = socket;
            req = {};
            // Set socket non block
            int tmpfd = socket->fd_get()->fd_;
            int flags = fcntl(tmpfd, F_GETFL);
            flags |= O_NONBLOCK;
            fcntl(tmpfd, F_SETFL, flags);
        }

        /**
         * \brief Start accepting connections on Client socket.
         */
        void operator()() final
        {
            if (!req)
                req = Request();
            char str_c[8192];
            int n = 0;
            try
            {
                n = sock_->recv(str_c, 8192);
                if (n <= 0)
                {
                    std::clog << "A socked has disconnect\n";
                    APM::global_connections_active--;
                    APM::global_connections_reading--;
                    event_register.unregister_ew(this);
                    return;
                }
            }
            catch (const std::exception& e)
            {
                std::clog << "A socked has disconnect\n";
                APM::global_connections_active--;
                APM::global_connections_reading--;
                event_register.unregister_ew(this);
                return;
            }

            // Return true if request is complete or ERROR. Return false if the
            // request is not complete
            bool is_complete = req.value()(str_c, n);
            if (is_complete)
            {
                std::clog << "We have a request ! \n" << str_c << std::endl;
                event_register.unregister_ew(this);
                shared_vhost v = dispatcher(req.value());
                Connection conn(sock_, v);
                v->get_apm().add_request();
                APM::global_connections_reading--;
                v->respond(req.value(), conn, 0, 0); // FIXME : Iterators
            }
            else
            {
                std::clog << "Current request is not complete...\n";
            }
        }

    private:
        /**
         * \brief Client socket.
         */
        shared_socket sock_;
        std::optional<Request> req;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
