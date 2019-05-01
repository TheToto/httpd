/**
 * \file events/client.hh
 * \brief ClientEW declaration.
 */

#pragma once

#include <exception>
#include <iostream>

#include "events/events.hh"
#include "events/register.hh"
#include "events/send.hh"
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

    static inline ssize_t test_complete(std::string& head)
    {
        if (head.find(http_crlf) == std::string::npos)
            return -1;
        size_t c_l = 0;
        size_t pos_size = head.find("Content-Length:");
        if (pos_size != std::string::npos)
        {
            pos_size = head.find_first_of(':', pos_size) + 1;
            pos_size = head.find_first_not_of(' ', pos_size);
            try
            {
                c_l = std::stoi(head.substr(pos_size));
            }
            catch(const std::exception& e)
            {
                c_l = 0;
            }
        }
        return c_l;
    }

    class ClientProxyEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a ClientProxyEW from a Client socket.
         */
        explicit ClientProxyEW(Connection conn)
            : EventWatcher(conn.backend_->fd_get()->fd_, EV_READ)
        {
            conn_ = conn;
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
                content_ += std::string(str_c, n);
                if (n <= 0)
                {
                    std::clog << "The backend has disconnect\n";
                    event_register.unregister_ew(this);
                    event_register.register_ew<SendResponseEW>(
                        conn_, error::bad_gateway(Request()));
                    return;
                }
            }
            catch (const std::exception& e)
            {
                std::clog << "The backend has disconnect\n";
                event_register.unregister_ew(this);
                event_register.register_ew<SendResponseEW>(
                    conn_, error::bad_gateway(Request()));
                return;
            }

            ssize_t c_l = test_complete(content_);
            size_t pos_head_end = content_.find(std::string(http_crlfx2)) + 4;
            if (c_l != -1 && content_.size() - pos_head_end - c_l == 0)
            {
                conn_.vhost_->apply_set_remove_header(false, content_, conn_);
                std::clog << "We have the backend response ! \n" << std::endl;
                event_register.unregister_ew(this);
                Response r(content_);
                event_register.register_ew<SendResponseEW>(conn_, r);
            }
            else
            {
                std::clog
                    << "Current response from backend is not complete...\n"
                        + content_ + "\n";
            }
        }

    private:
        /**
         * \brief Client socket.
         */
        Connection conn_;
        shared_socket sock_;
        std::string content_;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
