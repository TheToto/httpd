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
#include "config/health.hh"

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
        explicit ClientProxyEW(Connection conn, int timer_time)
            : EventWatcher(conn.backend_->fd_get()->fd_, EV_READ)
        {
            conn_ = conn;
            sock_ = conn.backend_;
            // Set socket non block
            int tmpfd = sock_->fd_get()->fd_;
            int flags = fcntl(tmpfd, F_GETFL);
            flags |= O_NONBLOCK;
            fcntl(tmpfd, F_SETFL, flags);

            if (timer_time != -1)
                init_timer_trans(timer_time);
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
                    stop_timer_trans();
                    event_register.unregister_ew(this);
                    if (conn_.is_health())
                    {
                        Health::health_callback(conn_, Response(""));//failed
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
            catch (const std::exception& e)
            {
                std::clog << "The backend has disconnect\n";
                stop_timer_trans();
                event_register.unregister_ew(this);
                if (conn_.is_health())
                {
                    Health::health_callback(conn_, Response(""));//failed
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

            ssize_t c_l = test_complete(content_);
            size_t pos_head_end = content_.find(std::string(http_crlfx2)) + 4;
            if (c_l != -1 && content_.size() - pos_head_end - c_l == 0)
            {
                conn_.vhost_->apply_set_remove_header(false, content_, conn_);
                std::clog << "We have the backend response ! \n" << std::endl;
                stop_timer_trans();
                event_register.unregister_ew(this);
                Response r(content_);

                if (conn_.is_health())
                {
                    Health::health_callback(conn_, r);//success or not
                    return;
                }

                if (r.get_status() != OK){
                    conn_.vhost_->conf_get().proxy_pass_.value()
                    .upstreams.set_health(- conn_.health_ - 1, false);
                }
                event_register.register_ew<SendResponseEW>(conn_, r);
            }
            else
            {
                std::clog
                    << "Current response from backend is not complete...\n"
                        + content_ + "\n";
            }
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
            auto ew = reinterpret_cast<ClientProxyEW*>(w->data);
            ew->stop_timer_trans(true);
        }

        void init_timer_trans(int remain)
        {
            if (conn_.vhost_.get()->conf_get().proxy_pass_->to_.has_value()) {
                timer_init_trans = true;
                ev_timer_init(&transaction_timer, abort_trans, remain, 0);
                transaction_timer.data = this;
                event_register.loop_get().register_timer_watcher(&transaction_timer);
            }
        }

    private:
        /**
         * \brief Client socket.
         */
        Connection conn_;
        shared_socket sock_;
        std::string content_;

        ev_timer transaction_timer;
        bool timer_init_trans = false;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
