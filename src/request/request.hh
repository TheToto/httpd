/**
 * \file request/request.hh
 * \brief Request declaration.
 */

#pragma once

#include "request/types.hh"

namespace http
{

    enum REQUEST_MODE : uint8_t
    {
        GET,
        POST,
        HEAD,
        ERROR
    };

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

        void set_mode(REQUEST_MODE mode);
        void set_uri(std::string uri);
        void set_host(std::string host);
    private:
        REQUEST_MODE mode;
        std::string uri;
        std::string host;
    };

    int get_mode(Request& r, const std::string& asked, int& cur);
    int get_host(Request& r, const std::string& asked, int& cur);
#include "request.hxx"
} // namespace http
