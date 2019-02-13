/**
 * \file request/response.hh
 * \brief Response declaration.
 */

#pragma once

#include <ctime>

#include "request/request.hh"
#include "request/types.hh"

namespace http
{
    /**
     * \struct Response
     * \brief Value object representing a response.
     */
    struct Response
    {
        explicit Response(const STATUS_CODE&);
        Response(const Request&, const STATUS_CODE& = STATUS_CODE::OK);

        Response() = default;
        Response(const Response&) = default;
        Response& operator=(const Response&) = default;
        Response(Response&&) = default;
        Response& operator=(Response&&) = default;
        ~Response() = default;
    private:
        STATUS_CODE status;
        std::time_t date;
    };
} // namespace http
