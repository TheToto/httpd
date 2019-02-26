#include <iostream>
#include <system_error>

#include "misc/fd.hh"
#include "request/error.hh"
#include "vhost/vhost-fail.hh"
#include "events/register.hh"
#include "events/send.hh"
namespace http
{
    void VHostFail::respond(const Request& r, Connection& conn,
                                  remaining_iterator, remaining_iterator)
    {
            auto mod = r.get_mode();
            std::string resp;
            if (mod == "ERROR METHOD")
                resp = error::method_not_allowed(r)();
            else if (mod == "OBSOLETE")
                resp = error::http_version_not_supported(r)();
            else if (mod == "UPGRADE")
                resp = error::upgrade_required(r)();
            else
                resp = error::bad_request()();
            event_register.register_ew<SendResponseEW>(conn.sock_,
            resp.c_str(), nullptr, 0);
    }
} // namespace http
