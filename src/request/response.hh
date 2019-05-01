/**
 * \file request/response.hh
 * \brief Response declaration.
 */

#pragma once

#include <ctime>

#include "misc/fd.hh"
#include "request/request.hh"
#include "request/types.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \struct Response
     * \brief Value object representing a response.
     */
    struct Response
    {
        explicit Response(const STATUS_CODE&);
        Response(const STATUS_CODE&, std::string realm);
        Response(std::string);
        Response(misc::shared_fd file, const STATUS_CODE& = STATUS_CODE::OK,
                 bool is_head = false, std::string list_dir = "",
                 std::string realm = "", std::string health = "");
        Response(const Request, misc::shared_fd file,
                 const STATUS_CODE& = STATUS_CODE::OK);
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

        void add_header(std::string name, std::string content)
        {
            size_t place_header = response_.find_first_of('\n') + 1;
            response_.insert(place_header,
                        name + ": " + content + http_crlf);
        }

        misc::shared_fd file_ = nullptr;
        size_t file_size_ = 0;

        STATUS_CODE get_status();

    private:
        STATUS_CODE status;
        std::string list_dir_ = "";
        std::string response_;
        std::string realm_ = "";
    };
} // namespace http
