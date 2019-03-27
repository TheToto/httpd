#pragma once

#include <string>

#include "request/types.hh"

namespace http
{
    struct APM
    {
        static size_t global_connections_active;
        static size_t global_connections_reading;
        static size_t global_connections_writing;
        static size_t global_requests_2xx;
        static size_t global_requests_4xx;
        static size_t global_requests_5xx;
        static size_t global_requests_nb;

        APM() = default;
        std::string get_json();
        void add_request();
        void add_request_final(STATUS_CODE status);


        size_t requests_2xx = 0;
        size_t requests_4xx = 0;
        size_t requests_5xx = 0;
        size_t requests_nb = 0;
    };

} // namespace http
