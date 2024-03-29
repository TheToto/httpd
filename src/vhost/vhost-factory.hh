/**
 * \file vhost/vhost-factory.hh
 * \brief VHostFactory
 */

#pragma once

#include <arpa/inet.h>
#include <optional>

#include "events/server.hh"
#include "misc/addrinfo/addrinfo.hh"

#include "socket/default-socket.hh"
#include "socket/ssl-socket.hh"

#include "vhost/dispatcher.hh"
#include "vhost/vhost-fail.hh"
#include "vhost/vhost-reverse-proxy.hh"
#include "vhost/vhost-static-file.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class VHostFactory
     * \brief Factory design pattern to create VHost.
     */
    class VHostFactory
    {
    public:
        /**
         * \brief Create a VHost object from a given VHostConfig.
         */
        static shared_vhost Fail()
        {
            auto conf = VHostConfig();
            auto vhost = new VHostFail(conf);
            return shared_vhost(vhost);
        }

        static shared_vhost Create(VHostConfig conf)
        {
            // Register vhost
            shared_vhost vhost;
            if (conf.proxy_pass_ != std::nullopt)
                vhost = shared_vhost(new VHostReverseProxy(conf));
            else
                vhost = shared_vhost(new VHostStaticFile(conf));

            // Create server socket
            shared_socket sock;
            misc::AddrInfoHint hints;
            hints.family(AF_UNSPEC).socktype(SOCK_STREAM);
            misc::AddrInfo res = misc::getaddrinfo(
                conf.ip_.c_str(), std::to_string(conf.port_).c_str(), hints);

            auto it = res.begin();
            for (; it != res.end(); it++)
            {
                if (conf.ssl_cert_ != "")
                    sock = shared_socket(new SSLSocket(it->ai_family,
                                                       it->ai_socktype,
                                                       it->ai_protocol,
                                                       vhost->ssl_ctx_get().get()));

                else
                    sock = shared_socket(new DefaultSocket(it->ai_family,
                                                           it->ai_socktype,
                                                           it->ai_protocol));

                if (sock->fd_get()->fd_ == -1)
                    continue;

                sock->setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
                sock->setsockopt(SOL_SOCKET, SO_REUSEPORT, 1);
                try
                {
                    sock->bind(it->ai_addr, it->ai_addrlen);
                    if (it->ai_family == AF_INET6)
                        sock->ipv6_set(true);
                    break;
                }
                catch (std::system_error&)
                {}
            }
            if (it == res.end())
                throw std::runtime_error("Can't bind " + conf.ip_);
            // listen
            sock->listen(128);
            event_register.register_ew<ServerEW>(sock);
            dispatcher.register_vhost(vhost);
            return vhost;
        }
    };
} // namespace http
