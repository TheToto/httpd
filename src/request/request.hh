/**
 * \file request/request.hh
 * \brief Request declaration.
 */

#pragma once

#include <map>
#include <vector>

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

        void set_mode(MOD mode_, int is_error=0)
        {
            if (is_error)
            {
                mode_error.push_back(mode_);
                erroring = 1;
            }
            else
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

        const MOD& get_mode() const
        {
            if (!erroring)
                return mode;
            return mode_error[0];
        }
        const MOD& get_proxy_errors() const
        {
            if (index_proxy_err == 0 || index_proxy_err > mode_error.size())
                return mode;
            return mode_error[index_proxy_err - 1];
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

        int get_mode_str(const std::string& asked, size_t& cur);
        char get_http_version(const std::string& asked, size_t& cur);
        bool parse_uri(std::string full_uri);
        void format_header_val(std::string& h_val);
        int get_headers_str(const std::string& asked, size_t& cur);
        char check_length();
        char check_httptwo();
        bool decode_uri(std::string&);
        bool operator()(const char* str, size_t n);

        bool is_head_ = false;

    private:
        MOD mode = MOD::ERROR;
        std::vector<MOD> mode_error;
        size_t index_proxy_err = 0;
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
} // namespace http
