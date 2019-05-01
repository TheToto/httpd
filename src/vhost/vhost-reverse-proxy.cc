/**
 * \file vhost/vhost-reverse-proxy.cc
 * \brief VhostReverseProxy implem.
 */

#include "vhost/vhost-reverse-proxy.hh"

#include <iostream>
#include <map>
#include <regex>
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
        event_register.register_ew<SendResponseEW>(conn, resp, is_head);
    }

    static inline bool conf_is_ipv6(std::string ip_proxy)
    {
        return ip_proxy.find(':') != std::string::npos;
    }

    void VHost::apply_set_remove_header(bool is_proxy, std::string& head, Connection conn)
    {
        std::set<std::string> h_remove;
        std::map<std::string, std::string> h_set;

        if (is_proxy)
        {
            h_remove = conf_.proxy_pass_.value().proxy_remove_header;
            h_set = conf_.proxy_pass_.value().proxy_set_header;
            // Host
            std::regex set_header("Host:(.*)\r\n");
            head = std::regex_replace(head, set_header, "");
            size_t place_header = head.find_first_of('\n') + 1;
            if (conf_is_ipv6(conf_.proxy_pass_.value().ip_))
                head.insert(place_header,
                        "Host: " + conf_.proxy_pass_.value().ipv6_port_ + http_crlf);
            else
                head.insert(place_header,
                        "Host: " + conf_.proxy_pass_.value().ip_port_ + http_crlf);
            // Forwarded
            if (head.find("Forwarded:") != std::string::npos)
            {
                std::regex for_header("Forwarded:([^\r]*)");
                std::string new_for("$&,for=" + conf_.ip_port_
                                    + ";host=" + conf_.server_name_);
                if (conn.sock_.get()->is_ipv6())
                    new_for = "$&,for=" + conf_.ipv6_port_
                                    + ";host=" + conf_.server_name_;
                if (conf_.ssl_cert_ != "")
                    new_for += ";proto=https";
                else
                    new_for += ";proto=http";
                head = std::regex_replace(head, for_header, new_for);
            }
            else
            {
                place_header = head.find_first_of('\n') + 1;
                std::string new_for("Forwarded: for=" + conf_.ip_port_
                                    + ";host=" + conf_.server_name_);
                if (conn.sock_.get()->is_ipv6())
                    new_for = "Forwarded: for=" + conf_.ipv6_port_
                                    + ";host=" + conf_.server_name_;
                if (conf_.ssl_cert_ != "")
                    new_for += ";proto=https";
                else
                    new_for += ";proto=http";
                head.insert(place_header, new_for + http_crlf);
            }
        }
        else
        {
            h_remove = conf_.proxy_pass_.value().remove_header;
            h_set = conf_.proxy_pass_.value().set_header;
        }
        for (auto r : h_remove)
        {
            std::regex del_header(r + ":(.*)\r\n");
            head = std::regex_replace(head, del_header, "");
        }
        for (auto r : h_set)
        {
            std::regex set_header(r.first + ":(.*)\r\n");
            head = std::regex_replace(head, set_header, "");
            size_t place_header = head.find_first_of('\n') + 1;
            head.insert(place_header, r.first + ": " + r.second + http_crlf);
        }
    }

    void VHostReverseProxy::respond(Request& request, Connection conn,
                                    remaining_iterator, remaining_iterator)
    {
        if (!request.is_erroring() && conf_.auth_basic_.has_value())
        {
            std::string auth = request.get_header("Authorization");
            if (auth == "")
            {
                send_response(conn,
                              error::proxy_authentication_required(
                                  request, conf_.auth_basic_.value()));
                return;
            }
            auto cur_1 = auth.find_first_of(' ');
            auto cur_2 = auth.find_first_not_of(' ', cur_1);
            auto len = auth.find_first_of(' ', cur_2) - cur_2;
            auth = auth.substr(cur_2, len);
            auto user = conf_.auth_basic_users_.value().begin();
            for (; user != conf_.auth_basic_users_.value().end(); user++)
            {
                std::cout << *user << std::endl;
                if (*user == auth)
                    break;
            }
            if (user == conf_.auth_basic_users_.value().end())
            {
                send_response(conn,
                              error::proxy_authentication_required(
                                  request, conf_.auth_basic_.value()));
                return;
            }
        }
        if (request.is_erroring())
        {
            auto mod = request.get_mode();
            if (mod == MOD::ERROR_URI_TOO_LONG)
                send_response(conn, error::uri_too_long());
            else if (mod == MOD::ERROR_PAYLOAD_TOO_LARGE)
                send_response(conn, error::payload_too_large());
            else if (mod == MOD::HEADER_FIELD_TOO_LARGE)
                send_response(conn, error::header_fields_too_large());
            return;
        }
        if (request.get_uri() == conf_.health_endpoint_)
        {
            Response resp(nullptr, STATUS_CODE::OK, request.is_head_, "", "",
                          apm.get_json());
            send_response(conn, resp, request.is_head_);
            return;
        }
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
        apply_set_remove_header(true, request.get_head(), conn);
        event_register.register_ew<SendProxyEW>(conn, request.rebuild());

        // 3. Recv backend header (new EventWatcher ClientProxy) and apply
        //   In ClientProxyEW

        // 4. Send backend response to client (Send EventWatcher)
        //   In ClientProxyEW to Send EventWatcher
    }

} // namespace http
