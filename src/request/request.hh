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
        HEAD
    };

    /**
     * \struct Request
     * \brief Value object representing a request.
     */
    struct Request
    {
        Request() = default;
        Request(const Request&) = default;
        Request& operator=(const Request&) = default;
        Request(Request&&) = default;
        Request& operator=(Request&&) = default;
        ~Request() = default;
    private:
        REQUEST_MODE mode;
        std::string uri;
        std::string host;
    };
} // namespace http
