/**
 * \file events/server.hh
 * \brief ServerEW declaration.
 */

#pragma once

#include "events/events.hh"
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
            : sock_(socket)
            , EventWatcher(socket.fd_get(), EV_READ)
        {
            // Set socket non block
            int tmpfd = socket.fd_get().fd_;
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
            while (1)
            {
                client_sock = sock_.accept(NULL, NULL);
                if (client_sock.fd_get())
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                    {
                        throw std::runtime_error;
                    }
                    break;
                }
                auto ew = ListenerEW(client_sock);
                event_register.register_ew(ew);
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
