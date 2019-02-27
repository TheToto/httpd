/**
 * \file request/request.hh
 * \brief Request declaration.
 */

#pragma once

#include <map>

#include "request/types.hh"

namespace http
{
    /**
     * \struct Request
     * \brief Value object representing a request.
     */
    struct Request
    {
        Request(std::string asked);
        Request() = default;
        Request(const Request&) = default;
        Request& operator=(const Request&) = default;
        Request(Request&&) = default;
        Request& operator=(Request&&) = default;
        ~Request() = default;

        void set_mode(std::string mode_)
        {
            mode = mode_;
        }
        void set_uri(std::string uri_)
        {
            uri = uri_;
        }
        void set_header(std::string name, std::string value)
        {
            headers[name] = value;
        }

        const std::string& get_mode() const
        {
            return mode;
        }
        const std::string& get_uri() const
        {
            return uri;
        }
        const std::string& get_version() const
        {
            return version;
        }
        const std::string& get_header(const std::string& name)
        {
            return headers[name];
        }
        const int& is_erroring() const
        {
            return erroring;
        }
        void set_erroring(const int& i)
        {
            erroring = i;
        }
        const size_t& get_length() const
        {
            return length;
        }
        void set_length(const int& i)
        {
            length = i;
        }

        int get_mode_str(const std::string& asked, int& cur);
        char get_http_version(const std::string& asked, int& cur);
        void parse_uri(std::string full_uri);
        void format_header_val(std::string& h_val);
        int get_headers_str(const std::string& asked, int& cur);
        char check_length();
        char check_httptwo();
        bool operator()(const char *str, size_t n);

    private:
        std::string mode = "";
        std::string src = "";
        std::string uri = "";
        std::string query = "";
        std::string version = "";
        std::string body = "";
        std::map<std::string, std::string> headers;
        int erroring = 0;
        size_t length = 0;
        bool headed = false;
        std::string head = "";
            };

    int get_mode_str(Request& r, const std::string& asked, int& cur);
    int get_headers_str(Request& r, const std::string& asked, int& cur);
    void format_header_val(std::string& h_val);
} // namespace http
