#include "apm.hh"

namespace http
{
    std::string APM::get_json()
    {
        nlohmann::json j;

        j["global_connections_active"] = global_connections_active;
        j["global_connections_reading"] = global_connections_reading;
        j["global_connections_writing"] = global_connections_writing;
        j["global_requests_2xx"] = global_requests_2xx;
        j["global_requests_4xx"] = global_requests_4xx;
        j["global_requests_5xx"] = global_requests_5xx;
        j["global_requests_nb"] = global_requests_nb;
        j["requests_2xx"] = requests_2xx;
        j["requests_4xx"] = requests_4xx;
        j["requests_5xx"] = requests_5xx;
        j["requests_nb"] = requests_nb;

        return j.dump();
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
