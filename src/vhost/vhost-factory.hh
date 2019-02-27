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
#include "misc/addrinfo/addrinfo.hh"

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
            
            shared_socket sock;

            misc::AddrInfoHint hints;
            hints.family(AF_UNSPEC).socktype(SOCK_STREAM);
            misc::AddrInfo res = misc::getaddrinfo(conf.ip_.c_str(),
                std::to_string(conf.port_).c_str(), hints);
            for (auto it = res.begin(); it != res.end(); it++)
            {
                sock = shared_socket(new DefaultSocket(
                    it->ai_family, it->ai_socktype, it->ai_protocol));
                if (sock->fd_get()->fd_ == -1)
                    continue;

                sock->setsockopt(SOL_SOCKET, SO_REUSEADDR, 1);
                try
                {
                    sock->bind(it->ai_addr, it->ai_addrlen);
                    break;
                }
                catch (std::system_error&)
                {}
            }

            // listen
            sock->listen(128);
            event_register.register_ew<ServerEW>(sock);

            dispatcher.register_vhost(vhost);
            return vhost;
        }
    };
} // namespace http
