/**
 * \file events/SendResponse.hh
 * \brief SendResponseEW declaration.
 */

#pragma once

#include <exception>
#include <iostream>

#include "events/events.hh"
#include "events/client.hh"
#include "events/register.hh"
#include "misc/socket.hh"
#include "request/error.hh"
#include "request/request.hh"
#include "socket/socket.hh"
#include "vhost/connection.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost.hh"
#include "misc/fd.hh"

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
        explicit SendResponseEW(shared_socket socket, std::string to_send, misc::shared_fd file, size_t file_size)
            : EventWatcher(socket->fd_get()->fd_, EV_WRITE)
        {
            sock_ = socket;
            sended_ = 0;
            to_send_ = to_send;
            file_ = file;
            file_size_ = file_size;
        }

        /**
         * \brief Start accepting connections on SendResponse socket.
         */
        void operator()() final
        {
            std::clog << "Sending response...\n";
            sock_->send(to_send_.c_str(), to_send_.size());
            if (file_)
            {
                off_t off = 0;
                sock_->sendfile(file_, off, file_size_);
            }
            // FIXME : IF EVERYTHING SEND
            // Unregister response, and register a normal listener
            event_register.unregister_ew(this);
            event_register.register_ew<ClientEW>(sock_);
            std::clog << "Response sent ! Listening for other request...\n";
        }

    private:
        /**
         * \brief SendResponse socket.
         */
        shared_socket sock_;
        std::string to_send_;
        misc::shared_fd file_;
        size_t file_size_;
        off_t sended_;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
