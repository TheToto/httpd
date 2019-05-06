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
        event_register.register_ew<SendResponseEW>(conn, resp, is_head);
    }

    void VHostFail::respond(Request& r, Connection conn, remaining_iterator,
                            remaining_iterator)
    {
        auto mod = r.get_mode();
        if (mod == MOD::ERROR_METHOD)
            send_response(conn, error::method_not_allowed(r));
        else if (mod == MOD::TIMEOUT_TRANSACTION)
            send_response(conn, error::timeout_transaction(r));
        else if (mod == MOD::TIMEOUT_TRANSACTION_PROXY)
            send_response(conn, error::timeout_transaction_proxy(r));
        else if (mod == MOD::TIMEOUT_KEEPALIVE)
            send_response(conn, error::timeout_keepalive(r));
        else if (mod == MOD::TIMEOUT_THROUGHPUT)
            send_response(conn, error::timeout_throughput(r));
        else if (mod == MOD::ERROR_MOVED_PERMANENTLY)
            send_response(conn, error::moved_permanently(r.get_uri()));
        else if (mod == MOD::OBSOLETE)
            send_response(conn, error::http_version_not_supported(r));
        else if (mod == MOD::UPGRADE)
            send_response(conn, error::upgrade_required(r));
        else if (mod == MOD::HEADER_FIELD_TOO_LARGE)
            send_response(conn, error::header_fields_too_large());
        else if (mod == MOD::ERROR_URI_TOO_LONG)
            send_response(conn, error::uri_too_long());
        else if (mod == MOD::ERROR_PAYLOAD_TOO_LARGE)
            send_response(conn, error::payload_too_large());
        else
            send_response(conn, error::bad_request());
    }
} // namespace http
