#include "vhost/vhost-static-file.hh"

#include <iostream>
#include <system_error>

#include "events/register.hh"
#include "events/send.hh"
#include "misc/fd.hh"
#include "request/error.hh"
namespace http
{
    static inline void send_response(Connection& conn, Response resp,
                                     bool is_head = false)
    {
        event_register.register_ew<SendResponseEW>(conn.sock_, resp, is_head);
    }

    static inline bool is_dir(std::string& path)
    {
        struct stat buf;
        try
        {
            sys::stat(path.c_str(), &buf);
        }
        catch (const std::exception& e)
        {
            return false;
        }
        return S_ISDIR(buf.st_mode);
    }

    void VHostStaticFile::respond(const Request& request, Connection& conn,
                                  remaining_iterator, remaining_iterator)
    {
        if (request.is_erroring())
        {
            auto mod = request.get_mode();
            if (mod == MOD::ERROR_METHOD)
                send_response(conn, error::method_not_allowed(request));
            else if (mod == MOD::OBSOLETE)
                send_response(conn, error::http_version_not_supported(request));
            else if (mod == MOD::UPGRADE)
                send_response(conn, error::upgrade_required(request));
            else
                send_response(conn, error::bad_request());
            return;
        }

        std::string path = this->conf_get().root_;

        path += request.get_uri();
        if (*(path.rbegin()) == '/')
            path += this->conf_get().default_file_;
        else if (is_dir(path))
            path += '/' + this->conf_get().default_file_;
        int fd = -1;
        try
        {
            fd = sys::open(path.c_str(), O_RDONLY);
        }
        catch (const std::system_error& e)
        {
            if (errno == ENOENT)
                send_response(conn, error::not_found(request));
            else
                send_response(conn, error::forbidden(request));
            return;
        }

        auto stream = std::make_shared<misc::FileDescriptor>(fd);

        Response resp(request, stream);

        send_response(conn, resp, request.is_head_);
    }
} // namespace http
