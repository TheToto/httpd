/**
 * \file vhost/vhost-factory.hh
 * \brief VHostFactory
 */

#include <arpa/inet.h>

#include "events/server.hh"
#include "socket/default-socket.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost-fail.hh"
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
            auto vhost = shared_vhost(new VHostStaticFile(conf));

            // Detect ip_protocol
            shared_socket sock;

            sockaddr_in ipv4;
            ipv4.sin_family = AF_INET;
            ipv4.sin_port = htons(conf.port_);
            if (inet_pton(AF_INET, conf.ip_.c_str(), &(ipv4.sin_addr)) > 0)
            {
                sock =
                    shared_socket(new DefaultSocket(AF_INET, SOCK_STREAM, 0));
                sock->setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
                sock->bind((sockaddr*)&ipv4, sizeof(sockaddr_in));
            }
            else
            {
                sockaddr_in6 ipv6;
                ipv6.sin6_family = AF_INET6;
                ipv6.sin6_port = htons(conf.port_);
                ipv6.sin6_flowinfo = 0;
                ipv6.sin6_scope_id = 0;
                if (inet_pton(AF_INET6, conf.ip_.c_str(), &(ipv6.sin6_addr))
                    > 0)
                {
                    vhost->set_ipv6(true);
                    sock = shared_socket(
                        new DefaultSocket(AF_INET6, SOCK_STREAM, 0));
                    sock->setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
                    sock->bind((sockaddr*)&ipv6, sizeof(sockaddr_in6));
                }
                else
                {
                    throw new std::runtime_error("Can't assign this ip : "
                                                 + conf.ip_);
                }
            }
            // listen
            sock->listen(128);
            event_register.register_ew<ServerEW>(sock);

            dispatcher.register_vhost(vhost);
            return vhost;
        }
    };
} // namespace http
