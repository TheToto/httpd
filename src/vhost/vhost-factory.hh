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
            sockaddr server;
            shared_socket sock;
            auto ip_protocol = AF_INET;

            sockaddr_in ipv4;
            ipv4.sin_family = ip_protocol;
            ipv4.sin_port = htons(conf.port_);
            if (inet_pton(ip_protocol, conf.ip_.c_str(), &(ipv4.sin_addr)) > 0)
            {
                sock = shared_socket(
                    new DefaultSocket(ip_protocol, SOCK_STREAM, 0));
                sock->bind((sockaddr*)&ipv4, sizeof(server));
            }
            else
            {
                sockaddr_in6 ipv6;
                ip_protocol = AF_INET6;
                ipv6.sin6_family = ip_protocol;
                ipv6.sin6_port = htons(conf.port_);
                if (inet_pton(ip_protocol, conf.ip_.c_str(), &(ipv6.sin6_addr))
                    > 0)
                {
                    sock = shared_socket(
                        new DefaultSocket(ip_protocol, SOCK_STREAM, 0));
                    sock->bind((sockaddr*)&ipv6, sizeof(server));
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
