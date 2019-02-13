/**
 * \file vhost/vhost-factory.hh
 * \brief VHostFactory
 */

#include "vhost/vhost-static-file.hh"
#include "vhost/vhost.hh"
#include "events/server.hh"
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

            auto sock = shared_socket(new DefaultSocket(AF_UNIX, SOCK_STREAM, 0));
            auto ew = ServerEW(sock);
            event_register.register_ew(ew);

            dispatcher.register_vhost(vhost);
        }
    };
} // namespace http
