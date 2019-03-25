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
        if (size_t pos_size = head.find("Content-Length:"))
        {
            pos_size = head.find_first_of(':', pos_size) + 1;
            pos_size = head.find_first_not_of(' ', pos_size);
            c_l = std::stoi(head.substr(pos_size));
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
                header_ += std::string(str_c, n);
                if (n <= 0)
                {
                    std::clog << "The backend has disconnect\n";
                    event_register.unregister_ew(this);
                    event_register.register_ew<SendResponseEW>(
                        conn_.sock_, error::bad_gateway(Request()));
                    return;
                }
            }
            catch (const std::exception& e)
            {
                std::clog << "The backend has disconnect\n";
                event_register.unregister_ew(this);
                event_register.register_ew<SendResponseEW>(
                    conn_.sock_, error::bad_gateway(Request()));
                return;
            }

            ssize_t is_complete = test_complete(header_);
            if (is_complete != -1)
            {
                conn_.vhost_->apply_set_remove_header(false, header_);
                size_t pos_head_end =
                    header_.find(std::string(http_crlfx2)) + 4;

                std::clog << "We have the backend response ! \n" << std::endl;
                event_register.unregister_ew(this);
                Response r(header_);
                r.file_ = conn_.backend_->fd_get();
                r.file_size_ = header_.size() - pos_head_end - is_complete;
                std::cout << "Content length : " << is_complete
                          << " head end : " << pos_head_end
                          << " remain : " << r.file_size_ << std::endl;
                event_register.register_ew<SendResponseEW>(conn_.sock_, r);
                std::cout << header_.substr(pos_head_end);
            }
            else
            {
                std::clog
                    << "Current response from backend is not complete...\n"
                        + header_ + "\n";
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
