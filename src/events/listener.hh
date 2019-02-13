/**
 * \file events/listener.hh
 * \brief ListenerEW declaration.
 */

#pragma once

#include <exception>
#include <iostream>

#include "events/events.hh"
#include "events/register.hh"
#include "socket/socket.hh"

namespace http
{
    /**
     * \class ListenerEW
     * \brief Workflow for listener socket.
     */
    class ListenerEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a ListenerEW from a listener socket.
         */
        explicit ListenerEW(shared_socket socket)
            : EventWatcher(socket->fd_get()->fd_, EV_READ)
        {
            sock_ = socket;
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
         * \brief Start accepting connections on listener socket.
         */
        void operator()() final
        {
            char* str_c[1000];
            std::cout << "[r]";
            int n = sock_->recv(str_c, 1000);
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
                    throw std::runtime_error("Wtf");
                }
                return;
            }
            std::cout << "socket client said: " << str_c;
            // response;
        }

    private:
        /**
         * \brief Listener socket.
         */
        shared_socket sock_;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
