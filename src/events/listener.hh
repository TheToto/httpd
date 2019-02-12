/**
 * \file events/listener.hh
 * \brief ListenerEW declaration.
 */

#pragma once

#include <iostream>

#include "events/events.hh"
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
            : sock_(socket)
            , EventWatcher(socket.fd_get(), EV_READ)
        {
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
            std::string str;
            std::cout << "[r]";
            int n = sock_->recv(str, 100, 0);
            if (n <= 0)
            {
                if (0 == n)
                {
                    // Disconnect
                    event_register.unregister_ew(&watcher_);
                }
                else
                {
                    throw std::runtime_error;
                }
                return;
            }
            std::cout << "socket client said: " << str;
            ;
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
