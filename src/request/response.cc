#include "request/response.hh"

#include <ctime>

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

    Response::Response(const Request, misc::shared_fd file,
                       const STATUS_CODE& code)
        : Response(file, code)
    {
        // Osef request ?
    }

    Response::Response(const Request,
                       const STATUS_CODE& code)
        : Response(nullptr, code)
    {}

    Response::Response(const STATUS_CODE& code)
        : Response(nullptr, code)
    {}

    Response::Response(misc::shared_fd file,
                       const STATUS_CODE& code)
        : status(code)
    {
        file_ = file;
        file_size_ = get_size(file);
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
    }
} // namespace http
