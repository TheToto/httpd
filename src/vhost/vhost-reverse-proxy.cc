/**
 * \file vhost/vhost-reverse-proxy.cc
 * \brief VhostReverseProxy implem.
 */

#include "vhost/vhost-reverse-proxy.hh"

#include "config/config.hh"
#include "request/request.hh"
#include "vhost/connection.hh"
#include "vhost/vhost.hh"

namespace http
{
    void VHostReverseProxy::respond(Request& request, Connection conn,
                                  remaining_iterator, remaining_iterator)
    {
        request = request;
        conn = conn;
        // 1. Create connection to backend.
        // 2. Recv client header and apply set/remove
        // 2. Send client request to backend (new EventWatcher ProxySend)
        // 3. Recv backend header (new EventWatcher ProxyRecv) and apply set/remove
        // 4. Send backend response to client (Send EventWatcher)
        // 5. Close connection to backend.
    }

} // namespace http
