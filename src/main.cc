#include <iostream>
#include <cstring>
#include "error/not-implemented.hh"
#include "vhost/vhost-factory.hh"
#include "config/config.hh"
#include "events/register.hh"
#include "events/server.hh"
#include "events/event-loop.hh"
#include "request/response.hh"

namespace http
{
    EventWatcherRegistry event_register = EventWatcherRegistry();
    Dispatcher dispatcher = Dispatcher();
}

static void stop_server(struct ev_loop* loop, ev_signal*, int)
{
    ev_break (loop, EVBREAK_ALL);
}

static int handle_t(char *argv[])
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
        return 2;
    }
}

static int handle_two(char *argv[])
{
    if (strcmp(argv[1], "-t") == 0)
    {
        std::clog << "Usage: spider [-t] file.JSON\n";
        return 2;
    }
    http::ServerConfig serv = http::parse_configuration(argv[1]);
    for (auto i : serv.VHosts_)
    {
        std::cout << "IP: " << i.ip_ << '\n'
                    << "PORT: " << i.port_ << '\n'
                    << "SERVER_NAME: " << i.server_name_ << '\n'
                    << "ROOT: " << i.root_ << '\n'
                    << "DEFAULT_FILE: " << i.default_file_ << '\n'
                    << '\n';
    }
    //http::Response a(http::NOT_FOUND);
    //std::cout << a() << '\n';

    auto vhost = http::VHostFactory::Create(serv.VHosts_[0]);

    auto loop = http::event_register.loop_get();
    ev_signal sigint_watcher;
    ev_signal_init (&sigint_watcher, stop_server, SIGINT);
    loop.register_sigint_watcher(&sigint_watcher);

    loop();
    return 1;
}

int main(int argc, char *argv[])
{
    switch (argc)
    {
        case 1:
            std::clog << "Usage: spider [-t] file.JSON\n";
            return 2;
        case 2:
             return handle_two(argv);
        case 3:
            return handle_t(argv);
        default:
            std::clog << "Usage: spider [-t] file.JSON\n";
            return 2;
    }
}

