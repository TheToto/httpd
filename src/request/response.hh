/**
 * \file request/response.hh
 * \brief Response declaration.
 */

#pragma once

#include <ctime>

#include "misc/fd.hh"
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

        Response(misc::shared_fd file, const STATUS_CODE& = STATUS_CODE::OK,
                 bool is_head = false);
        Response(const Request, misc::shared_fd file,
                 const STATUS_CODE& = STATUS_CODE::OK);
        Response(std::string list_dir);
        Response(const Request, const STATUS_CODE& = STATUS_CODE::OK);
        // get the body of the response and its length

        Response() = default;
        Response(const Response&) = default;
        Response& operator=(const Response&) = default;
        Response(Response&&) = default;
        Response& operator=(Response&&) = default;
        ~Response() = default;

        const std::string& operator()() const;
        std::string sup_body();

        misc::shared_fd file_ = nullptr;
        size_t file_size_ = 0;

    private:
        STATUS_CODE status;
        std::string list_dir_ = "";
        std::string response_;
    };
} // namespace http
