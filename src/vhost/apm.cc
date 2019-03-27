#include "apm.hh"

namespace http
{
    std::string APM::get_json()
    {
        // Create JSON here
        return "{}";
    }

    void APM::add_request()
    {
        requests_nb++;
        global_requests_nb++;
    }

    void APM::add_request_final(STATUS_CODE status)
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
}
