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

    Response::Response(const STATUS_CODE& code)
        : status(code)
    {
        date = std::time(0);
        version_ = "HTTP/1.1";

        auto pcode = statusCode(code);

        response_ += version_;
        response_ += " ";
        response_ += std::to_string(code);
        response_ += " ";
        response_ += pcode.second;
        response_ += http_crlf;

        response_ += "Content-Length: 0";
        response_ += http_crlf;

        response_ += "Date: ";
        char tab[80] = {0};
        response_ += std::string(get_time(tab));
        response_ += http_crlf;

        response_ += http_crlf;
    }

    Response::Response(const Request& request, const STATUS_CODE& code)
        : status(code)
    {
        date = std::time(0);
        version_ = request.get_version();

        auto pcode = statusCode(code);

        response_ = "HTTP/1.1 ";
        response_ += std::to_string(code);
        response_ += " ";
        response_ += pcode.second;
        response_ += http_crlf;
        response_ += "Content-Length: 0";
        response_ += http_crlf;
        response_ += "Date: ";
        char tab[80] = {0};
        response_ += std::string(get_time(tab));
        response_ += http_crlf;

        response_ += http_crlf;
    }

    Response::Response(const Request& request, size_t& size,
                       const STATUS_CODE& code)
        : status(code)
    {
        date = std::time(0);
        version_ = request.get_version();

        auto pcode = statusCode(code);

        response_ = "HTTP/1.1 ";
        response_ += std::to_string(code);
        response_ += " ";
        response_ += pcode.second;
        response_ += http_crlf;
        response_ += "Content-Length: ";
        response_ += std::to_string(size + 2);
        response_ += http_crlf;
        response_ += "Date: ";
        char tab[80] = {0};
        response_ += std::string(get_time(tab));
        response_ += http_crlf;

        response_ += http_crlf;
    }
} // namespace http
