/**
 * \file events/SendResponse.hh
 * \brief SendResponseEW declaration.
 */

#pragma once

#include <exception>
#include <iostream>

#include "events/client.hh"
#include "events/events.hh"
#include "events/register.hh"
#include "misc/fd.hh"
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
     * \class SendResponseEW
     * \brief Workflow for SendResponse socket.
     */
    class SendResponseEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a SendResponseEW from a SendResponse socket.
         */
        explicit SendResponseEW(Connection conn, Response resp,
                                bool is_head = false)
            : EventWatcher(conn.sock_->fd_get()->fd_, EV_WRITE)
        {
            conn_ = conn;
            resp_ = resp;
            APM::global_connections_writing++;
            conn.vhost_->get_apm().add_request_final(resp.get_status());
            sock_ = conn.sock_;
            sended_ = 0;
            to_send_ = resp();
            if (!is_head)
            {
                file_ = resp.file_;
                file_size_ = resp.file_size_;
            }
            else
            {
                file_ = nullptr;
                file_size_ = 0;
            }
            sended_header_ = 0;
        }

        virtual ~SendResponseEW() override
        {
            APM::global_connections_writing--;
        }

        /**
         * \brief Start accepting connections on SendResponse socket.
         */
        void operator()() final
        {
            std::clog << "Sending response... currently "
                      << sended_ + sended_header_ << " of "
                      << file_size_ + to_send_.size() << "\n";
            if (sended_header_ < to_send_.size())
            {
                try
                {
                    std::cout << "debug : " << sock_.get()->fd_get().get()->fd_ << std::endl;
                    sended_header_ +=
                        sock_->send(to_send_.c_str() + sended_header_,
                                    to_send_.size() - sended_header_);
                    if (sended_header_ < to_send_.size())
                        return;
                }
                catch (const std::exception&)
                {
                    std::clog << "Connection aborded ! 1\n";
                    APM::global_connections_active--;
                    event_register.unregister_ew(this);
                    return;
                }
            }
            if (file_ && file_size_ > 0)
            {
                try
                {
                    sock_->sendfile(file_, sended_, file_size_ - sended_);
                }
                catch (const std::exception&)
                {
                    std::clog << "Connection aborded ! 2\n";
                    APM::global_connections_active--;
                    event_register.unregister_ew(this);
                    return;
                }
            }
            // Unregister response, and register a normal listener
            size_t sended = sended_;
            if (sended >= file_size_)
            {
                event_register.unregister_ew(this);
                event_register.register_ew<ClientEW>(sock_);
                std::clog << "Response sent ! Listening for other request...\n";
            }
        }

    private:
        /**
         * \brief SendResponse socket.
         */
        size_t sended_header_;
        shared_socket sock_;
        std::string to_send_;
        misc::shared_fd file_;
        size_t file_size_;
        off_t sended_;
        Connection conn_;
        Response resp_;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
