/**
 * \file request/request.hh
 * \brief Request declaration.
 */

#pragma once

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
        void set_host(std::string host_)
        {
            host = host_;
        }

        const std::string& get_mode()
        {
            return mode;
        }
        const std::string& get_uri()
        {
            return uri;
        }
        const std::string& get_host()
        {
            return host;
        }
    private:
        std::string mode;
        std::string uri;
        std::string host;
    };

    int get_mode_str(Request& r, const std::string& asked, int& cur);
    int get_host_str(Request& r, const std::string& asked, int& cur);
#include "request.hxx"
} // namespace http
