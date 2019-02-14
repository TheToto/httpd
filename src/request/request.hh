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

    private:
        std::string mode;
        std::string uri;
        std::string version;
        std::map<std::string, std::string> headers;
    };

    int get_mode_str(Request& r, const std::string& asked, int& cur);
    int get_headers_str(Request& r, const std::string& asked, int& cur);
} // namespace http
