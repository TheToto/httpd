/**
 * \file events/server.hh
 * \brief ServerEW declaration.
 */

#pragma once

#include "events/events.hh"
#include "events/listener.hh"
#include "events/register.hh"
#include "socket/socket.hh"

namespace http
{
    /**
     * \class ServerEW
     * \brief Workflow for listener socket.
     */
    class ServerEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a ServerEW from a listener socket.
         */
        explicit ServerEW(shared_socket socket)
            : EventWatcher(socket->fd_get()->fd_, EV_READ)
        {
            sock_ = socket;
            // Set socket non block
            int tmpfd = socket->fd_get()->fd_;
            sys::fcntl_wrapper(tmpfd, O_NONBLOCK);

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
            while (1)
            {
                auto client_sock = sock_->accept(NULL, NULL);
                if (*(client_sock->fd_get()) == -1)
                {
                    break;
                }
                std::clog << "Accept a new socket\n";
                event_register.register_ew<ListenerEW>(client_sock);
            }
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
