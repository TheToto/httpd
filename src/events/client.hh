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
            if (serv_conf.keep_alive.has_value())
                init_timer_keepalive();
        }

        /**
         * \brief Start accepting connections on Client socket.
         */
        void operator()() final
        {
            is_running = true;
            if (!req)
            {
                req = Request();
                if (serv_conf.throughput_val.has_value())
                    init_timer_throughput();
            }
            char str_c[8192];
            int n = 0;
            try
            {
                n = sock_->recv(str_c, 8192);

                if (timer_init_keepalive)
                    stop_timer_keepalive();
                if (n <= 0)
                {
                    std::clog << "A socked has disconnect\n";
                    APM::global_connections_active--;
                    APM::global_connections_reading--;
                    stop_timer_trans();
                    event_register.unregister_ew(this);
                    is_running = false;
                    return;
                }

                if (serv_conf.transaction.has_value() && !timer_init_trans)
                    init_timer_trans();
                data_size += n;
            }
            catch (const std::exception& e)
            {
                std::clog << "A socked has disconnect\n";
                APM::global_connections_active--;
                APM::global_connections_reading--;
                stop_timer_trans();
                event_register.unregister_ew(this);
                is_running = false;
                return;
            }

            // Return true if request is complete or ERROR. Return false if the
            // request is not complete
            bool is_complete = req.value()(str_c, n);
            if (is_complete)
            {
                stop_timer_throughput();
                std::clog << "We have a request ! \n" << req.value().get_head() << std::endl;
                stop_timer_trans();
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
            is_running = false;
        }

        void init_timer_trans()
        {
            // TODO : Set config value
            std::cout << "Init timer 1!" << std::endl;
            timer_init_trans = true;
            ev_timer_init(&transaction_timer, abort_trans,
                          serv_conf.transaction.value(), 0);
            transaction_timer.data = this;
            event_register.loop_get().register_timer_watcher(&transaction_timer);
        }

        void init_timer_keepalive()
        {
            // TODO : Set config value
            std::cout << "Init timer 2!" << std::endl;
            timer_init_keepalive = true;
            ev_timer_init(&keepalive_timer, abort_keepalive,
                          serv_conf.keep_alive.value(), 0);
            keepalive_timer.data = this;
            event_register.loop_get().register_timer_watcher(&keepalive_timer);
        }

        void init_timer_throughput()
        {
            // TODO : Set config value
            std::cout << "Init timer 3!" << std::endl;
            timer_init_throughput = true;
            ev_timer_init(&throughput_timer, callback_throughput, 0.01, 0);
            throughput_timer.data = this;
            event_register.loop_get().register_timer_watcher(&throughput_timer);
        }

        void stop_timer_trans(bool cut = false)
        {
            if (!serv_conf.transaction.has_value())
                return;
            std::cout << "STOP timer 1!" << std::endl;
            if (timer_init_keepalive)
                stop_timer_keepalive();
            timer_init_trans = false;
            ev_timer_stop(event_register.loop_get().loop, &transaction_timer);
            if (cut)
            {
                std::cout << "ERROR timer 1!" << std::endl;
                if (timer_init_throughput)
                    stop_timer_throughput();
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
            if (!serv_conf.keep_alive.has_value())
                return;
            std::cout << "STOP timer 2!" << std::endl;
            timer_init_keepalive = false;
            ev_timer_stop(event_register.loop_get().loop, &keepalive_timer);
            if (cut)
            {
                std::cout << "ERROR timer 2!" << std::endl;
                if (timer_init_throughput)
                    stop_timer_throughput();
                if (timer_init_trans)
                    stop_timer_trans();
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

        void stop_timer_throughput()
        {
            if (!serv_conf.throughput_val.has_value())
                return;
            std::cout << "STOP timer 3!" << std::endl;
            if (timer_init_keepalive)
                stop_timer_keepalive();
            timer_init_throughput = false;
            ev_timer_stop(event_register.loop_get().loop, &throughput_timer);
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

        static void callback_throughput(struct ev_loop*, ev_timer *w, int)
        {
            if (!serv_conf.throughput_val.has_value())
                return;
            std::cout << "CALLBACK timer 3!" << std::endl;
            auto ew = reinterpret_cast<ClientEW*>(w->data);
            ew->stop_timer_throughput();
            if (ew->data_size / serv_conf.throughput_time.value() 
                        < serv_conf.throughput_val.value())
            {
                std::cout << "ERROR timer 3!" << std::endl;
                if (ew->timer_init_trans)
                    ew->stop_timer_trans();
                if (!ew->is_running) {
                    shared_socket save_sock = ew->sock_;
                    shared_vhost v = Dispatcher::get_fail();
                    Request r;
                    r.set_mode(MOD::TIMEOUT_THROUGHPUT);
                    Connection conn(save_sock, v, r);
                    APM::global_connections_reading--;
                    v->respond(r, conn, 0, 0);
                }
                event_register.unregister_ew(ew);
                return;
            }
            ew->init_timer_throughput();
        }

    private:
        /**
         * \brief Client socket.
         */
        ev_timer transaction_timer;
        ev_timer keepalive_timer;
        ev_timer throughput_timer;
        bool timer_init_trans = false;
        bool timer_init_keepalive = false;
        bool timer_init_throughput = false;
        bool is_running = false;

        size_t data_size;

        shared_socket sock_;
        std::optional<Request> req;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
