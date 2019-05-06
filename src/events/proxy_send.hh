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
            init_timer_trans();
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
                    stop_timer_trans();
                    event_register.unregister_ew(this);
                    if (conn_.is_health())
                    {
                        Health::health_callback(conn_, Response(""));
                        return;
                    }
                    if (conn_.vhost_->conf_get().proxy_pass_->method_ == fail_robin){
                        Health::health_callback(conn_, Response(""));
                        conn_.vhost_->respond(conn_.req_, conn_, 0, 0);
                        return;
                    }
                    event_register.register_ew<SendResponseEW>(
                        conn_, error::bad_gateway(Request()));
                    return;
                }
            }
            // Unregister response, and register a normal listener
            int remain = -1;
            if (timer_init_trans)
                remain = ev_timer_remaining(event_register.loop_get().loop, &transaction_timer);
            stop_timer_trans();
            event_register.unregister_ew(this);
            event_register.register_ew<ClientProxyEW>(conn_, remain);
            std::clog << "Request sent to backend ! Waiting for response...\n";
        }

        void stop_timer_trans(bool cut = false)
        {
            if (!timer_init_trans)
                return;
            timer_init_trans = false;
            ev_timer_stop(event_register.loop_get().loop, &transaction_timer);
            if (cut)
            {
                shared_socket save_sock = sock_;
                event_register.unregister_ew(this);
                if (conn_.is_health())
                {
                    Health::health_callback(conn_, Response(""));
                    return;
                }
                shared_vhost v = Dispatcher::get_fail();
                Request r;
                r.set_mode(MOD::TIMEOUT_TRANSACTION_PROXY);
                Connection conn(save_sock, v, r);
                v->respond(r, conn, 0, 0);
            }
        }

        static void abort_trans(struct ev_loop*, ev_timer *w, int)
        {
            auto ew = reinterpret_cast<SendProxyEW*>(w->data);
            ew->stop_timer_trans(true);
        }

        void init_timer_trans()
        {
            if (conn_.vhost_.get()->conf_get().proxy_pass_->to_.has_value() && !timer_init_trans) {
                timer_init_trans = true;
                ev_timer_init(&transaction_timer, abort_trans, 
                        conn_.vhost_.get()->conf_get().proxy_pass_->to_.value(), 0);
                transaction_timer.data = this;
                event_register.loop_get().register_timer_watcher(&transaction_timer);
            }
        }

    private:
        /**
         * \brief SendResponse socket.
         */
        shared_socket sock_;
        size_t sended_;
        Connection conn_;
        std::string to_send_;

        ev_timer transaction_timer;
        bool timer_init_trans = false;

        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
