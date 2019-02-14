#include "vhost/vhost-static-file.hh"
#include "request/error.hh"

namespace http
{
    static inline void send_response(Connection& conn, std::string& response)
    {
        send(conn.sock_.send(response, response.length()); 
    }

    void VHostStaticFile::respond(const Request& request, Connection& conn, remaining_iterator begin,
                     remaining_iterator end)
    {
        //
        if (request.get_mode() == "ERROR")
        {
            auto resp = error::bad_request();
            send_response(conn, resp());
            return;
        }
        
        std::string path = this->conf_get().root_;
        path += request.get_uri();
        auto stream = sys::open(path.c_str(), O_RDONLY);

        if (stream == -1)
        {
            if (errno == ENOENT)
            {
                auto resp = error::not_found(request);
                send_response(conn, resp());
            }
            else
            {
                auto resp = error::forbidden(request);
                send_response(conn, resp());
            }
        }
        else
        {
            struct stat buffer;
            auto stat = sys::fstat(stream, &buffer);
            size_t size = buffer.st_size;
            auto resp(request, size);
            std::string body = resp();
            conn.sock_.send(body, body.length());
            conn.sock_.sendfile(stream, 0, size);
            conn.sock_.send(http_crlf, 2);
        }
    }
}
