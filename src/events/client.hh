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
            sock_ = socket;
            req = {};
            // Set socket non block
            int tmpfd = socket->fd_get()->fd_;
            int flags = fcntl(tmpfd, F_GETFL);
            flags |= O_NONBLOCK;
            fcntl(tmpfd, F_SETFL, flags);

            /*struct sockaddr_in sin;
            socklen_t len = sizeof(sin);
            if (getsockname(socket, (struct sockaddr*)&sin, &len) != -1)
                port_ = ntohs(sin.sin_port);*/
        }

        /**
         * \brief Start accepting connections on Client socket.
         */
        void operator()() final
        {
            if (req)
                req = Request();
            char str_c[10000];
            int n = sock_->recv(str_c, 10000);
            if (n <= 0)
            {
                if (0 == n)
                {
                    // Disconnect
                    std::clog << "A socked has disconnect\n";
                    event_register.unregister_ew(this);
                }
                else
                {
                    throw std::runtime_error("Invalid recv");
                }
                return;
            }
            /*
            // Return true if request is complete or ERROR. Return false if the request is not complete
            bool is_complete = req.continue_parsing(str_c);
            if (is_complete)
            {
                std::clog << "We have a request ! \n" << str_c << std::endl;
                event_register.unregister_ew(this);
                Connection con(sock_);
                shared_vhost v = dispatcher(req);
                v->respond(req, con, 0, 0); // FIXME : Iterators
            }
            else
            {
                std::clog << "Current request is not complete...\n"
            }
            */
            // DELETE FOLLOWING
            Request req_old(str_c);
            // IF IS FULL... stop listening for data
            std::clog << "We have a request ! \n" << str_c << std::endl;
            event_register.unregister_ew(this);
            Connection con(sock_);
            shared_vhost v = dispatcher(req_old);
            v->respond(req_old, con, 0, 0); /* FIXME : Iterators */
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
