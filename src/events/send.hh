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
                    ssize_t tmpsend = sock_->send(to_send_.c_str() + sended_header_,
                                                  to_send_.size() - sended_header_);
                    init_timer_throughput();
                    sended_header_ += tmpsend;
                    datasize += tmpsend;

                    if (sended_header_ < to_send_.size())
                        return;
                }
                catch (const std::exception&)
                {
                    std::clog << "Connection aborded ! 1\n";
                    APM::global_connections_active--;
                    stop_timer_thoughput();
                    event_register.unregister_ew(this);
                    return;
                }
            }
            if (file_ && file_size_ > 0)
            {
                try
                {
                    ssize_t tmpsend = sock_->sendfile(file_, sended_, file_size_ - sended_);
                    datasize += tmpsend;
                }
                catch (const std::exception&)
                {
                    std::clog << "Connection aborded ! 2\n";
                    APM::global_connections_active--;
                    stop_timer_thoughput();
                    event_register.unregister_ew(this);
                    return;
                }
            }
            // Unregister response, and register a normal listener
            size_t sended = sended_;
            if (sended >= file_size_)
            {
                stop_timer_thoughput();
                event_register.unregister_ew(this);
                if (check_keep_alive())
                {
                    event_register.register_ew<ClientEW>(sock_);
                    std::clog << "Response sent ! Listening for other request...\n";
                }
                else
                {
                    std::clog << "Response sent ! Closing connection...\n";
                }
            }
        }

        bool check_keep_alive()
        {
            if (conn_.req_.get_header("Connection") == "close")
                return false;
            return !(resp_.get_status() >= 500
                     || resp_.get_status() == 400
                     || resp_.get_status() == 413
                     || resp_.get_status() == 414
                     || resp_.get_status() == 431
                     || resp_.get_status() == 408);
        }

        void init_timer_throughput()
        {
            if (serv_conf.throughput_time.has_value() && !timer_init_throughput) {
                timer_init_throughput = true;
                ev_periodic_init(&throughput_timer, callback_throughput, 0, serv_conf.throughput_time.value(), 0);
                throughput_timer.data = this;
                event_register.loop_get().register_period_watcher(&throughput_timer);
            }
        }

        void stop_timer_thoughput() {
            if (timer_init_throughput) {
                timer_init_throughput = false;
                ev_periodic_stop(event_register.loop_get().loop, &throughput_timer);
            }
        }

        void check_timer_throughput() {
            if (datasize / serv_conf.throughput_time.value()
                < serv_conf.throughput_val.value()) {
                stop_timer_thoughput();
                event_register.unregister_ew(this);
            }
            datasize = 0;
        }


        static void callback_throughput(struct ev_loop*, ev_periodic *w, int)
        {
            auto ew = reinterpret_cast<SendResponseEW*>(w->data);
            ew->check_timer_throughput();
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

        ev_periodic throughput_timer;
        bool timer_init_throughput = false;
        size_t datasize = 0;
        /**
         * \brief Port on which the socket is listening.
         */
        uint16_t port_;
    };
} // namespace http
