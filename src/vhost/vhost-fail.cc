#include "vhost/vhost-fail.hh"

#include <iostream>
#include <system_error>

#include "events/register.hh"
#include "events/send.hh"
#include "misc/fd.hh"
#include "request/error.hh"
#include "request/types.hh"

namespace http
{
    static inline void send_response(Connection& conn, Response resp,
                                     bool is_head = false)
    {
        event_register.register_ew<SendResponseEW>(conn.sock_, resp, is_head);
    }

    void VHostFail::respond(const Request& r, Connection& conn,
                            remaining_iterator, remaining_iterator)
    {
        auto mod = r.get_mode();
        if (mod == MOD::ERROR_METHOD)
            send_response(conn, error::method_not_allowed(r));
        else if (mod == MOD::OBSOLETE)
            send_response(conn, error::http_version_not_supported(r));
        else if (mod == MOD::UPGRADE)
            send_response(conn, error::upgrade_required(r));
        else
            send_response(conn, error::bad_request());
    }
} // namespace http
