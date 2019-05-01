/**
 * \file events/SendResponse.hh
 * \brief SendResponseEW declaration.
 */

#pragma once

#include <exception>
#include <iostream>

#include "events/client.hh"
#include "events/events.hh"
#include "events/proxy_client.hh"
#include "events/register.hh"
#include "misc/fd.hh"
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
     * \class SendResponseEW
     * \brief Workflow for SendResponse socket.
     */
    class SendProxyEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a SendResponseEW from a SendResponse socket.
         */
        explicit SendProxyEW(Connection conn, std::string to_send)
            : EventWatcher(conn.backend_->fd_get()->fd_, EV_WRITE)
        {
            conn_ = conn;
            sock_ = conn.backend_;
            sended_ = 0;
            to_send_ = to_send;

            int tmpfd = sock_->fd_get()->fd_;
            int flags = fcntl(tmpfd, F_GETFL);
            flags |= O_NONBLOCK;
            fcntl(tmpfd, F_SETFL, flags);
        }

        /**
         * \brief Start accepting connections on SendResponse socket.
         */
        void operator()() final
        {
            std::clog << "Sending request to backend... currently " << sended_
                      << " of " << to_send_.size() << "\n";
            if (sended_ < to_send_.size())
            {
                try
                {
                    sended_ += sock_->send(to_send_.c_str() + sended_,
                                           to_send_.size() - sended_);
                    if (sended_ < to_send_.size())
                        return;
                }
                catch (const std::exception&)
                {
                    std::clog << "Connection aborded with backend !\n";
                    event_register.unregister_ew(this);
                    if (conn_.is_health())
                    {
                        Health::health_callback(conn_, Response(""));
                        return;
                    }
                    event_register.register_ew<SendResponseEW>(
                        conn_, error::bad_gateway(Request()));
                    return;
                }
            }
            // Unregister response, and register a normal listener
            event_register.unregister_ew(this);
            event_register.register_ew<ClientProxyEW>(conn_);
            std::clog << "Request sent to backend ! Waiting for response...\n";
        }

    private:
        /**
         * \brief SendResponse socket.
         */
        shared_socket sock_;
        size_t sended_;
        Connection conn_;
        std::string to_send_;

        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
