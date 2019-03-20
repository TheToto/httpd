/**
 * \file misc/html.cc
 * \brief Generate HTML errors
 */
#include <filesystem>
#include <string>

#include "request/types.hh"

namespace misc
{
    class Html
    {
    public:
        static std::string generate_html(std::string title, std::string body)
        {
            std::string html;
            html = "<!DOCTYPE html>\n<html>\n"
                   "<head>\n<meta charset=utf-8>\n<title>"
                + title
                + "</title>\n"
                  "</head>\n<body>\n"
                + body + "</body>\n</html>\n";
            return html;
        }

        static std::string generate_dir(std::string path)
        {
            std::string body;
            body += "<li>\n";
            for (auto& p : std::filesystem::directory_iterator(path))
            {
                body += "<ul>";
                body += "<a href=\"";
                body += p.path().filename();
                if (p.is_directory())
                    body += "/";
                body += "\">";
                body += p.path().filename();
                if (p.is_directory())
                    body += "/";
                body += "</a>";
                body += "</ul>\n";
            }
            body += "</li>\n";
            return generate_html("Index of " + path.substr(1), body);
        }

        static std::string generate_error(http::STATUS_CODE& status)
        {
            if (status == http::STATUS_CODE::NOT_FOUND)
                return generate_html("404 Not Found",
                                     "<h3>404 error</h3><p>The requested URL "
                                     "was not found on this server.</p>");
            if (status == http::STATUS_CODE::FORBIDDEN)
                return generate_html(
                    "403 Forbidden",
                    "<h3>403 error</h3><p>You don't have permission to access "
                    "to this URL on this server.</p>");
            if (status == http::STATUS_CODE::BAD_REQUEST)
                return generate_html(
                    "400 Bad Request",
                    "<h3>400 error</h3><p>You request is malformed.</p>");
            return "";
        }
    };
} // namespace misc
