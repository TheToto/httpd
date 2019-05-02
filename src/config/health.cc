#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "config/health.hh"
#include "events/proxy_send.hh"
#include "config/config.hh"
#include "error/not-implemented.hh"
#include "events/event-loop.hh"
#include "events/register.hh"
#include "events/server.hh"
#include "misc/openssl/ssl.hh"
#include "request/response.hh"
#include "vhost/apm.hh"
#include "vhost/vhost-factory.hh"

namespace http {
    void Health::check_alive(shared_vhost vhost)
    {
        for (size_t i = 0; i < vhost->conf_get().proxy_pass_.value().upstreams.vector.size(); i++){
            shared_socket sock;
            Upstream& cur = vhost->conf_get().proxy_pass_.value().upstreams.vector[i];
            misc::AddrInfoHint hints;
            hints.family(AF_UNSPEC).socktype(SOCK_STREAM);
            misc::AddrInfo res = misc::getaddrinfo(
                    cur.ip_.c_str(),
                    std::to_string(cur.port_).c_str(), hints);
            auto it = res.begin();
            for (; it != res.end(); it++)
            {
                sock = shared_socket(new DefaultSocket(
                        it->ai_family, it->ai_socktype, it->ai_protocol));
                if (sock->fd_get()->fd_ == -1)
                    continue;

                sock->setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
                sock->setsockopt(SOL_SOCKET, SO_REUSEPORT, 1);
                try
                {
                    sock->connect(it->ai_addr, it->ai_addrlen);
                    break;
                }
                catch (std::system_error&)
                {}
            }
            if (it == res.end())
            {
                cur.alive = false;
                continue;
            }
            Connection connection(sock, vhost,i);
            event_register.register_ew<SendProxyEW>(connection, vhost->conf_get().proxy_pass_.value().upstreams.build_health(cur.health_, cur.ip_port_));
        }
    }

    void Health::check_all_vhost() {
        for (shared_vhost vhost : dispatcher.getVhosts())
        {
            if (vhost->conf_get().proxy_pass_.has_value()) {
                const methods & tmp = vhost->conf_get().proxy_pass_.value().method_;
                if (tmp == fail_robin || tmp == failover)
                    Health::check_alive(vhost);
            }
        }
    }

    void Health::health_callback(Connection &conn, Response resp) {
        conn.vhost_.get()->conf_get().proxy_pass_.value().upstreams.set_health(conn.health_, resp.get_status() == STATUS_CODE::OK);
    }
}