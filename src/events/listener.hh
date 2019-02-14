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
            std::cout << "socket client said: \n" << str_c << std::endl;
            // response;
            //Resquest req(str_c);
            //Connection con(sock_);
            //dispatcher.dispatch(res, con);
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
