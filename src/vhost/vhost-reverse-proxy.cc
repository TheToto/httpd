/**
 * \file vhost/vhost-reverse-proxy.cc
 * \brief VhostReverseProxy implem.
 */

#include "vhost/vhost-reverse-proxy.hh"

#include <iostream>
#include <map>
#include <set>
#include <sstream>

#include "config/config.hh"
#include "events/proxy_send.hh"
#include "events/register.hh"
#include "events/send.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "request/request.hh"
#include "request/response.hh"
#include "socket/default-socket.hh"
#include "vhost/connection.hh"
#include "vhost/vhost.hh"

namespace http
{
    static inline void send_response(Connection& conn, Response resp,
                                     bool is_head = false)
    {
        event_register.register_ew<SendResponseEW>(conn.sock_, resp, is_head);
    }

    void VHost::apply_set_remove_header(bool is_proxy, std::string& head)
    {
        std::set<std::string> h_remove;
        std::map<std::string, std::string> h_set;

        if (is_proxy)
        {
            h_remove = conf_.proxy_pass_.value().proxy_remove_header;
            h_set = conf_.proxy_pass_.value().proxy_set_header;
        }
        else
        {
            h_remove = conf_.proxy_pass_.value().proxy_remove_header;
            h_set = conf_.proxy_pass_.value().proxy_set_header;
        }
        for (auto r : h_remove)
        {
            if (size_t i = head.find(r))
            {
                size_t end = head.find(std::string(http_crlf));
                head.erase(i, end);
            }
        }
        for (auto r : h_set)
        {
            if (size_t i = head.find(r.first))
            {
                size_t end = head.find(std::string(http_crlf));
                head.erase(i, end);
            }
            size_t req_line = head.find(std::string(http_crlf));
            head.insert(req_line + 2,
                        r.first + ": " + r.second + std::string(http_crlf));
        }
        return;
    }

    void VHostReverseProxy::respond(Request& request, Connection conn,
                                    remaining_iterator, remaining_iterator)
    {
        // 1. Create connection to backend.
        shared_socket sock;

        misc::AddrInfoHint hints;
        hints.family(AF_UNSPEC).socktype(SOCK_STREAM);
        misc::AddrInfo res = misc::getaddrinfo(
            conf_.proxy_pass_.value().ip_.c_str(),
            std::to_string(conf_.proxy_pass_.value().port_).c_str(), hints);
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
            send_response(conn, error::bad_gateway(request));
            return;
        }
        conn.backend_ = sock;
        // 2. Send client request to backend (new EventWatcher ProxySend)
        apply_set_remove_header(true, request.get_head());
        event_register.register_ew<SendProxyEW>(conn, request.rebuild());

        // 3. Recv backend header (new EventWatcher ClientProxy) and apply
        //   In ClientProxyEW

        // 4. Send backend response to client (Send EventWatcher)
        //   In ClientProxyEW to Send EventWatcher
    }

} // namespace http
