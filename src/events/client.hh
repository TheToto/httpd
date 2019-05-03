/**
 * \file events/client.hh
 * \brief ClientEW declaration.
 */

#pragma once

#include <exception>
#include <iostream>

#include "events/events.hh"
#include "events/register.hh"
#include "misc/socket.hh"
#include "request/error.hh"
#include "request/request.hh"
#include "socket/socket.hh"
#include "vhost/apm.hh"
#include "vhost/connection.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost.hh"
#include "events/register.hh"
namespace http
{
    /**
     * \class ClientEW
     * \brief Workflow for client socket.
     */
    class ClientEW : public EventWatcher
    {
    public:
        /**
         * \brief Create a ClientEW from a Client socket.
         */
        explicit ClientEW(shared_socket socket)
            : EventWatcher(socket->fd_get()->fd_, EV_READ)
        {
            APM::global_connections_reading++;
            sock_ = socket;
            req = {};
            // Set socket non block
            int tmpfd = socket->fd_get()->fd_;
            int flags = fcntl(tmpfd, F_GETFL);
            flags |= O_NONBLOCK;
            fcntl(tmpfd, F_SETFL, flags);
            init_timer_keepalive();
        }

        /**
         * \brief Start accepting connections on Client socket.
         */
        void operator()() final
        {
            if (!req)
                req = Request();

            char str_c[8192];
            int n = 0;
            try
            {
                n = sock_->recv(str_c, 8192);

                stop_timer_keepalive();
                init_timer_trans();
                if (n <= 0)
                {
                    std::clog << "A socked has disconnect\n";
                    APM::global_connections_active--;
                    APM::global_connections_reading--;
                    stop_all_timer();
                    event_register.unregister_ew(this);
                    return;
                }
                data_size += n;
            }
            catch (const std::exception& e)
            {
                std::clog << "A socked has disconnect\n";
                APM::global_connections_active--;
                APM::global_connections_reading--;
                stop_all_timer();
                event_register.unregister_ew(this);
                return;
            }

            // Return true if request is complete or ERROR. Return false if the
            // request is not complete
            bool is_complete = req.value()(str_c, n);
            if (is_complete)
            {
                std::clog << "We have a request ! \n" << req.value().get_head() << std::endl;
                stop_all_timer();
                event_register.unregister_ew(this);
                shared_vhost v = dispatcher(req.value());
                Connection conn(sock_, v, req.value());
                v->get_apm().add_request();
                APM::global_connections_reading--;
                v->respond(req.value(), conn, 0, 0); // FIXME : Iterators
            }
            else
            {
                std::clog << "Current request is not complete...\n";
            }
        }

        void init_timer_trans()
        {
            if (serv_conf.transaction.has_value()) {
                std::cout << "Init timer 1!" << std::endl;
                timer_init_trans = true;
                ev_timer_init(&transaction_timer, abort_trans,
                              serv_conf.transaction.value(), 0);
                transaction_timer.data = this;
                event_register.loop_get().register_timer_watcher(&transaction_timer);
            }
            if (serv_conf.throughput_time.has_value()) {
                timer_init_throughput = true;
                ev_periodic_init(&throughput_timer, callback_throughput, 0, serv_conf.throughput_time.value(), 0);
                throughput_timer.data = this;
                event_register.loop_get().register_period_watcher(&throughput_timer);
            }
        }

        void init_timer_keepalive()
        {
            if (serv_conf.keep_alive.has_value()) {
                timer_init_keepalive = true;
                ev_timer_init(&keepalive_timer, abort_keepalive,
                              serv_conf.keep_alive.value(), 0);
                keepalive_timer.data = this;
                event_register.loop_get().register_timer_watcher(&keepalive_timer);
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
                stop_all_timer();

                shared_socket save_sock = sock_;
                event_register.unregister_ew(this);
                shared_vhost v = Dispatcher::get_fail();
                Request r;
                r.set_mode(MOD::TIMEOUT_TRANSACTION);
                Connection conn(save_sock, v, r);
                APM::global_connections_reading--;
                v->respond(r, conn, 0, 0);
            }
        }

        void stop_timer_keepalive(bool cut = false)
        {
            if (!timer_init_keepalive)
                return;
            timer_init_keepalive = false;
            ev_timer_stop(event_register.loop_get().loop, &keepalive_timer);
            if (cut)
            {
                stop_all_timer();

                shared_socket save_sock = sock_;
                event_register.unregister_ew(this);
                shared_vhost v = Dispatcher::get_fail();
                Request r;
                r.set_mode(MOD::TIMEOUT_KEEPALIVE);
                Connection conn(save_sock, v, r);
                APM::global_connections_reading--;
                v->respond(r, conn, 0, 0);
            }
        }

        void check_timer_throughput() {
            if (data_size / serv_conf.throughput_time.value()
                < serv_conf.throughput_val.value()) {
                stop_all_timer();

                shared_socket save_sock = sock_;
                shared_vhost v = Dispatcher::get_fail();
                Request r;
                r.set_mode(MOD::TIMEOUT_THROUGHPUT);
                Connection conn(save_sock, v, r);
                APM::global_connections_reading--;
                v->respond(r, conn, 0, 0);

                event_register.unregister_ew(this);
            }
            data_size = 0;
        }

        void stop_all_timer()
        {
            if (timer_init_throughput) {
                timer_init_throughput = false;
                ev_periodic_stop(event_register.loop_get().loop, &throughput_timer);
            }
            if (timer_init_trans) {
                timer_init_trans = false;
                ev_timer_stop(event_register.loop_get().loop, &transaction_timer);
            }
            if (timer_init_keepalive) {
                timer_init_keepalive = false;
                ev_timer_stop(event_register.loop_get().loop, &keepalive_timer);
            }
        }

        static void abort_trans(struct ev_loop*, ev_timer *w, int)
        {
            auto ew = reinterpret_cast<ClientEW*>(w->data);
            ew->stop_timer_trans(true);
        }

        static void abort_keepalive(struct ev_loop*, ev_timer *w, int)
        {
            auto ew = reinterpret_cast<ClientEW*>(w->data);
            ew->stop_timer_keepalive(true);
        }

        static void callback_throughput(struct ev_loop*, ev_periodic *w, int)
        {
            auto ew = reinterpret_cast<ClientEW*>(w->data);
            ew->check_timer_throughput();
        }

    private:
        /**
         * \brief Client socket.
         */
        ev_timer transaction_timer;
        ev_timer keepalive_timer;
        ev_periodic throughput_timer;
        bool timer_init_trans = false;
        bool timer_init_keepalive = false;
        bool timer_init_throughput = false;

        size_t data_size;

        shared_socket sock_;
        std::optional<Request> req;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
