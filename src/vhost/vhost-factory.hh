/**
 * \file vhost/vhost-factory.hh
 * \brief VHostFactory
 */

#include <arpa/inet.h>

#include "vhost/dispatcher.hh"
#include "vhost/vhost-fail.hh"
#include "vhost/vhost-static-file.hh"
#include "vhost/vhost-reverse-proxy.hh"
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

            shared_vhost vhost;
            if (/* FIXME : conf.proxy_pass*/false)
                vhost = shared_vhost(new VHostReverseProxy(conf));
            else
                vhost = shared_vhost(new VHostStaticFile(conf));

            dispatcher.register_vhost(vhost);
            return vhost;
        }
    };
} // namespace http
