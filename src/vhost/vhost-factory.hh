/**
 * \file vhost/vhost-factory.hh
 * \brief VHostFactory
 */

#include <arpa/inet.h>

#include "events/server.hh"
#include "socket/default-socket.hh"
#include "vhost/dispatcher.hh"
#include "vhost/vhost-static-file.hh"
#include "vhost/vhost.hh"
#include "vhost/vhost-fail.hh"
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

            auto sock =
                shared_socket(new DefaultSocket(AF_INET, SOCK_STREAM, 0));
            // bind
            sockaddr_in server;
            server.sin_family = AF_INET;

            // Ues AF_INET6 for IPv6
            if (inet_pton(AF_INET, conf.ip_.c_str(), &(server.sin_addr)) != 0)
                inet_pton(AF_INET6, conf.ip_.c_str(), &(server.sin_addr));
            server.sin_port = htons(conf.port_);

            sock->bind((sockaddr*)&server, sizeof(server));
            // listen
            sock->listen(128);
            event_register.register_ew<ServerEW>(sock);

            dispatcher.register_vhost(vhost);
            return vhost;
        }
    };
} // namespace http
