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
qsd     * \brief Value object representing a response.
     */
    struct Response
    {
        explicit Response(const STATUS_CODE&);

        Response(const Request&, const STATUS_CODE& = STATUS_CODE::OK);
        Response(const Request&, size_t& size,
                 const STATUS_CODE& = STATUS_CODE::OK);
        // get the body of the response and its length

        Response() = default;
        Response(const Response&) = default;
        Response& operator=(const Response&) = default;
        Response(Response&&) = default;
        Response& operator=(Response&&) = default;
        ~Response() = default;

        const std::string& operator()() const;

    private:
        STATUS_CODE status;
        std::time_t date;
        std::string version_;

        std::string response_;
    };
} // namespace http
