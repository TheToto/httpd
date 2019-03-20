#include "request/response.hh"

#include <ctime>

#include "misc/html.hh"

namespace http
{
    const std::string& Response::operator()() const
    {
        return response_;
    }

    static char* get_time(char* ptr)
    {
        auto i = time(0);
        strftime(ptr, 80, "%a, %d %b %G %T GTM", gmtime(&i));
        return ptr;
    }

    static inline size_t get_size(misc::shared_fd& file)
    {
        if (file == nullptr)
            return 0;
        struct stat buffer;
        sys::fstat(*file, &buffer);
        return buffer.st_size;
    }

    Response::Response(const Request r, misc::shared_fd file,
                       const STATUS_CODE& code)
        : Response(file, code, r.is_head_)
    {}

    Response::Response(const Request r, const STATUS_CODE& code)
        : Response(nullptr, code, r.is_head_)
    {}

    Response::Response(const STATUS_CODE& code)
        : Response(nullptr, code)
    {}

    Response::Response(std::string list_dir)
        : list_dir_(list_dir)
    {
        Response(nullptr, STATUS_CODE::OK);
    }

    Response::Response(misc::shared_fd file, const STATUS_CODE& code,
                       bool is_head)
        : file_(file)
        , status(code)
    {
        auto add_body = sup_body();
        if (file != nullptr)
            file_size_ = get_size(file);
        else
            file_size_ = add_body.size();

        auto pcode = statusCode(code);

        response_ = "HTTP/1.1 ";
        response_ += std::to_string(code);
        response_ += " ";
        response_ += pcode.second;
        response_ += http_crlf;
        response_ += "Content-Length: ";
        response_ += std::to_string(file_size_);
        response_ += http_crlf;
        response_ += "Date: ";
        char tab[80] = {0};
        response_ += std::string(get_time(tab));
        response_ += http_crlf;

        response_ += "Connection: close";
        response_ += http_crlf;

        response_ += http_crlf;

        if (file == nullptr && !is_head)
        {
            response_ += add_body;
            file_size_ = 0;
        }
        else if (is_head)
            file_size_ = 0;
    }

    std::string Response::sup_body()
    {
        if (list_dir_ != "")
            return misc::Html::generate_dir(list_dir_);
        if (status != STATUS_CODE::OK)
            return misc::Html::generate_error(status);
        return "";
    }
} // namespace http
