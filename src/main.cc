#include <iostream>
#include <cstring>
#include "error/not-implemented.hh"
#include "config/config.hh"

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
    throw http::NotImplemented();
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

