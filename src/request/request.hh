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

        const std::string& get_mode()
        {
            return mode;
        }
        const std::string& get_uri()
        {
            return uri;
        }
        const std::string& get_header(const std::string& name)
        {
            return headers[name];
        }
    private:
        std::string mode;
        std::string uri;
        std::map<std::string, std::string> headers;
    };

    int get_mode_str(Request& r, const std::string& asked, int& cur);
    int get_headers_str(Request& r, const std::string& asked, int& cur);
#include "request.hxx"
} // namespace http
