#pragma once

#include "vhost/vhost.hh"
#include "request/response.hh"

namespace http {
    class Health {
    public:
        static void check_all_vhost();
        static void check_alive(shared_vhost vhost);
        static void health_callback(Connection& conn, Response resp);
    };
}

