/**
 * \file vhost/vhost-factory.hh
 * \brief VHostFactory
 */

#include "vhost/vhost-static-file.hh"
#include "vhost/vhost.hh"
#include "socket/default-socket.hh"
#include "vhost/dispatcher.hh"
#include "events/server.hh"
#include <arpa/inet.h>
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
        static shared_vhost Create(VHostConfig conf)
        {
            auto vhost = shared_vhost(new VHostStaticFile(conf));

            auto sock = shared_socket(new DefaultSocket(AF_INET, SOCK_STREAM, 0));
            // bind
            sockaddr_in server;
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = INADDR_ANY;
            server.sin_port = htons(/*conf.port_*/8000);
            sock->bind((sockaddr*)&server, sizeof(server));
            // listen
            sock->listen(3);
            event_register.register_ew<ServerEW>(sock);

            dispatcher.register_vhost(vhost);
            return vhost;
        }
    };
} // namespace http
