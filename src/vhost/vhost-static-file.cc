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
        event_register.register_ew<SendResponseEW>(conn, resp, is_head);
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

    static inline void file_exists(std::string& path)
    {
        struct stat buf;
        sys::stat(path.c_str(), &buf);
        if (!S_ISREG(buf.st_mode))
        {
            errno = ENOENT;
            throw std::logic_error("Can't open a dir");
        }
    }

    void VHostStaticFile::respond(Request& request, Connection conn,
                                  remaining_iterator, remaining_iterator)
    {
        if (!request.is_erroring() && conf_.auth_basic_.has_value())
        {
            std::string auth = request.get_header("Authorization");
            if (auth == "")
            {
                send_response(conn,
                              error::unauthorized(request,
                                  conf_.auth_basic_.value()));
                return;
            }
            auto cur_1 = auth.find_first_of(' ');
            auto cur_2 = auth.find_first_not_of(' ', cur_1);
            auto len = auth.find_first_of(' ', cur_2) - cur_2;
            auth = auth.substr(cur_2, len);
            auto user = conf_.auth_basic_users_.value().begin();
            for (; user != conf_.auth_basic_users_.value().end(); user++)
            {
                std::cout << *user << std::endl;
                if (*user == auth)
                    break;
            }
            if (user == conf_.auth_basic_users_.value().end())
            {
                send_response(conn,
                              error::unauthorized(request,
                                                    conf_.auth_basic_.value()));
                return;
            }
        }
        if (request.is_erroring())
        {
            auto mod = request.get_mode();
            if (mod == MOD::ERROR_METHOD)
                send_response(conn, error::method_not_allowed(request));
            else if (mod == MOD::OBSOLETE)
                send_response(conn, error::http_version_not_supported(request));
            else if (mod == MOD::UPGRADE)
                send_response(conn, error::upgrade_required(request));
            else if (mod == MOD::ERROR_URI_TOO_LONG)
                send_response(conn, error::uri_too_long());
            else if (mod == MOD::ERROR_PAYLOAD_TOO_LARGE)
                send_response(conn, error::payload_too_large());
            else if (mod == MOD::HEADER_FIELD_TOO_LARGE)
                send_response(conn, error::header_fields_too_large());
            else
                send_response(conn, error::bad_request());
            return;
        }
        if (request.get_uri() == conf_.health_endpoint_)
        {
            Response resp(nullptr, STATUS_CODE::OK, request.is_head_, "", "",
                          apm.get_json());
            send_response(conn, resp, request.is_head_);
            return;
        }
        std::string path = this->conf_get().root_;

        path += request.get_uri();
        std::string dir_path = path;
        if (*(path.rbegin()) == '/')
        {
            path += this->conf_get().default_file_;
        }
        else if (is_dir(path))
        {
            path += '/' + this->conf_get().default_file_;
            dir_path = "";
        }
        else
        {
            dir_path = "";
        }
        int fd = -1;
        try
        {
            file_exists(path);
            fd = sys::open(path.c_str(), O_RDONLY);
        }
        catch (const std::exception& e)
        {
            if (errno == ENOENT)
            {
                if (conf_.auto_index_ && dir_path != "" && is_dir(dir_path))
                {
                    Response resp(nullptr, STATUS_CODE::OK, request.is_head_,
                                  dir_path);
                    send_response(conn, resp);
                }
                else
                    send_response(conn, error::not_found(request));
            }
            else
            {
                send_response(conn, error::forbidden(request));
            }
            return;
        }

        auto stream = std::make_shared<misc::FileDescriptor>(fd);

        Response resp(request, stream);

        send_response(conn, resp, request.is_head_);
    }
} // namespace http
