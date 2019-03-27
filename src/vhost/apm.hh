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
        std::string get_json()
        {
            // Create JSON here
            return "{}";
        }
        void add_request()
        {
            requests_nb++;
            global_requests_nb++;
        }

        void add_request_final(STATUS_CODE status)
        {
            if (status < 300)
            {
                global_requests_2xx++;
                requests_2xx++;
            }
            else if (status < 500)
            {
                global_requests_4xx++;
                requests_4xx++;
            }
            else
            {
                global_requests_5xx++;
                requests_5xx++;
            }
        }

        size_t requests_2xx = 0;
        size_t requests_4xx = 0;
        size_t requests_5xx = 0;
        size_t requests_nb = 0;
    };

} // namespace http