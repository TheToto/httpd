#include "vhost/vhost-static-file.hh"

#include <system_error>

#include "misc/fd.hh"
#include "request/error.hh"

namespace http
{
    static inline void send_response(Connection& conn, std::string& response)
    {
        conn.sock_->send(response.c_str(), response.length());
    }

    void VHostStaticFile::respond(const Request& request, Connection& conn,
                                  remaining_iterator, remaining_iterator)
    {
        if (request.get_mode() == "ERROR")
        {
            auto resp = error::bad_request()();
            send_response(conn, resp);
            return;
        }

        std::string path = this->conf_get().root_;
        path += request.get_uri();
        if (*(path.rbegin()) == '/')
            path += this->conf_get().default_file_;
        int fd = -1;
        try
        {
            fd = sys::open(path.c_str(), O_RDONLY);
        }
        catch (const std::system_error& e)
        {
            if (e.code() == std::errc::no_such_file_or_directory)
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
        std::string body = resp();
        conn.sock_->send(body.c_str(), body.length());
        off_t off = 0;
        conn.sock_->sendfile(stream, off, size);
        conn.sock_->send(http_crlf, 2);
    }
} // namespace http
