#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "events/proxy_send.hh"
#include "config/config.hh"
#include "error/not-implemented.hh"
#include "events/event-loop.hh"
#include "events/register.hh"
#include "events/server.hh"
#include "misc/openssl/ssl.hh"
#include "request/response.hh"
#include "vhost/apm.hh"
#include "vhost/vhost-factory.hh"
#include "config/health.hh"

namespace http
{
    EventWatcherRegistry event_register = EventWatcherRegistry();
    Dispatcher dispatcher = Dispatcher();
    ServerConfig serv_conf;
    size_t APM::global_connections_active = 0;
    size_t APM::global_connections_reading = 0;
    size_t APM::global_connections_writing = 0;
    size_t APM::global_requests_2xx = 0;
    size_t APM::global_requests_4xx = 0;
    size_t APM::global_requests_5xx = 0;
    size_t APM::global_requests_nb = 0;
} // namespace http

static void stop_server(struct ev_loop* loop, ev_signal*, int)
{
    ev_break(loop, EVBREAK_ALL);
    std::clog << "Closing server..." << std::endl;
    EVP_cleanup();
}

static void continue_server(struct ev_loop*, ev_signal*, int)
{}

static void periodic_cb(struct ev_loop*, ev_periodic*, int)
{
    std::cout << "Checking VHost upstreams !" << std::endl;
    http::Health::check_all_vhost();
}

static int launch_thread(int i)
{
    for (auto conf : http::serv_conf.VHosts_)
    {
        std::clog << "Setup " << conf.server_name_ << " vhost.\n";
        http::VHostFactory::Create(conf);
    }

    auto loop = http::event_register.loop_get();

    ev_signal sigint_watcher;
    ev_signal_init(&sigint_watcher, stop_server, SIGINT);
    loop.register_sigint_watcher(&sigint_watcher);

    ev_signal sigpipe_watcher;
    ev_signal_init(&sigpipe_watcher, continue_server, SIGPIPE);
    loop.register_sigint_watcher(&sigpipe_watcher);

    ev_periodic health_back;
    ev_periodic_init(&health_back, periodic_cb, 0, 11, 0);
    loop.register_period_watcher(&health_back);

    std::clog << "Server launched !" << std::endl << "Process " << i << std::endl;
    loop();
    return 0;
}

static int launch_server(char* path)
{
    http::serv_conf = http::parse_configuration(path);
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    std::vector<int> pids;

    if (http::serv_conf.nb_workers == 1)
        return launch_thread(-1);

    for (int i = 0; i < http::serv_conf.nb_workers; i++)
    {
        int pid = fork();

        if (pid == 0)
        {
            return launch_thread(i);
        }
        else
        {
            pids.push_back(pid);
        }
    }
    int waitstatus = 0;
    for (int i = 0; i < http::serv_conf.nb_workers; i++)
        waitpid(pids[i], &waitstatus, 0);
    return 0;
}

static int handle_t(char* argv[])
{
    if (strcmp(argv[1], "-t") == 0)
    {
        try
        {
            return http::test_file(argv[2]);
        }
        catch (const std::exception& e)
        {
            std::clog << "An exception was caught while testing JSON:"
                      << e.what() << '\n';
            return 1;
        }
    }
    else if (strcmp(argv[2], "-t") == 0)
    {
        try
        {
            return http::test_file(argv[1]);
        }
        catch (const std::exception& e)
        {
            std::clog << "An exception was caught while testing JSON:\n"
                      << e.what() << '\n';
            return 1;
        }
    }
    else
    {
        std::clog << "Usage: spider [-t] file.JSON\n";
        return 1;
    }
}

static int handle_two(char* argv[])
{
    if (strcmp(argv[1], "-t") == 0)
    {
        std::clog << "Usage: spider [-t] file.JSON\n";
        return 1;
    }
    return launch_server(argv[1]);
}

int main(int argc, char* argv[])
{
    switch (argc)
    {
    case 1:
        std::clog << "Usage: spider [-t] file.JSON\n";
        return 1;
    case 2:
        return handle_two(argv);
    case 3:
        return handle_t(argv);
    default:
        std::clog << "Usage: spider [-t] file.JSON\n";
        return 1;
    }
}
