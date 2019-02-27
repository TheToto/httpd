#include "vhost/vhost-static-file.hh"

#include <iostream>
#include <system_error>

#include "events/register.hh"
#include "events/send.hh"
#include "misc/fd.hh"
#include "request/error.hh"
namespace http
{
    static inline void send_response(Connection& conn, std::string& response)
    {
        event_register.register_ew<SendResponseEW>(conn.sock_, response.c_str(),
                                                   nullptr, 0);
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
            std::string resp;
            if (mod == "ERROR METHOD")
                resp = error::method_not_allowed(request)();
            else if (mod == "OBSOLETE")
                resp = error::http_version_not_supported(request)();
            else if (mod == "UPGRADE")
                resp = error::upgrade_required(request)();
            else
                resp = error::bad_request()();
            send_response(conn, resp);
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
            {
                auto resp = error::not_found(request)();
                send_response(conn, resp);
            }
            else
            {
                auto resp = error::forbidden(request)();
                send_response(conn, resp);
            }
            return;
        }

        auto stream = std::make_shared<misc::FileDescriptor>(fd);
        struct stat buffer;
        sys::fstat(*stream, &buffer);
        size_t size = buffer.st_size;
        Response resp(request, size);
        std::string head = resp();
        if (request.get_mode() == "HEAD")
            stream = nullptr;
        event_register.register_ew<SendResponseEW>(conn.sock_, head, stream,
                                                   size);
    }
} // namespace http
