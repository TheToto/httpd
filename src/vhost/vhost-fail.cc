#include <iostream>
#include <system_error>

#include "misc/fd.hh"
#include "request/error.hh"
#include "vhost/vhost-fail.hh"
#include "events/register.hh"
#include "events/send.hh"
namespace http
{
    void VHostFail::respond(const Request&, Connection& conn,
                                  remaining_iterator, remaining_iterator)
    {
        auto resp = error::bad_request()();
        event_register.register_ew<SendResponseEW>(conn.sock_,
            resp.c_str(), nullptr, 0);
    }
} // namespace http
