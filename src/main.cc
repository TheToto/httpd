#include <cstring>
#include <iostream>

#include "config/config.hh"
#include "error/not-implemented.hh"
#include "events/event-loop.hh"
#include "events/register.hh"
#include "events/server.hh"
#include "request/response.hh"
#include "vhost/vhost-factory.hh"

#include "misc/openssl/ssl.hh"

namespace http
{
    EventWatcherRegistry event_register = EventWatcherRegistry();
    Dispatcher dispatcher = Dispatcher();

} // namespace http

static void stop_server(struct ev_loop* loop, ev_signal*, int)
{
    ev_break(loop, EVBREAK_ALL);
    std::clog << "Closing server..." << std::endl;
    EVP_cleanup();
}

static void continue_server(struct ev_loop*, ev_signal*, int)
{}

static int launch_server(char* path)
{
    http::ServerConfig serv = http::parse_configuration(path);

    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    for (auto conf : serv.VHosts_)
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

    std::clog << "Server launched !" << std::endl;
    loop();
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
