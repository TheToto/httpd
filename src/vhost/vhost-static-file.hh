/**
 * \file vhost/vhost-static-file.hh
 * \brief VHostStaticFile declaration.
 */

#pragma once

#include "config/config.hh"
#include "request/request.hh"
#include "vhost/connection.hh"
#include "vhost/vhost.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "socket/default-socket.hh"
#include "vhost/dispatcher.hh"
#include "events/register.hh"
#include "events/server.hh"

namespace http
{
    /**
     * \class VHostStaticFile
     * \brief VHost serving static files.
     */
    class VHostStaticFile : public VHost
    {
    public:
        friend class VHostFactory;
        virtual ~VHostStaticFile() = default;

    private:
        /**
         * \brief Constructor called by the factory.
         *
         * \param config VHostConfig virtual host configuration.
         */
        explicit VHostStaticFile(const VHostConfig& conf)
            : VHost(conf)
        {
            shared_socket sock;

            misc::AddrInfoHint hints;
            hints.family(AF_UNSPEC).socktype(SOCK_STREAM);
            misc::AddrInfo res = misc::getaddrinfo(
                conf.ip_.c_str(), std::to_string(conf.port_).c_str(), hints);
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
                    sock->bind(it->ai_addr, it->ai_addrlen);
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
        }

    public:
        /**
         * \brief Send response.
         *
         * \param req Request.
         * \param conn Connection.
         * \param begin remaining_iterator of received data.
         * \param end remaining_iterator of received data.
         *
         * Note that these iterators will only be useful starting from SRPS.
         */
        void respond(Request&, Connection, remaining_iterator,
                     remaining_iterator) final;
    };
} // namespace http
