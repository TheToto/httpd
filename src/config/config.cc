#include "config/config.hh"

#include "error/not-implemented.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Winline"
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wlogical-op-parentheses"
#endif
#include <json.hpp>
#include <iostream>//clog
#include <stdexcept>//invalid_argument
#include <string>//to_string
#include <fstream>//ifstream
#pragma GCC diagnostic pop

using json = nlohmann::json;

namespace http
{
    ServerConfig parse_configuration(const std::string& path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open())
            throw std::invalid_argument("cannot open file\n");
        json j = json::parse(ifs);
        ServerConfig serv;

        for (auto i : j)
        {
            json tmp(i);
            if (tmp.size() != 4)
                throw std::invalid_argument("invalid json file: not a \
json or wrong architecture");
            std::string ip = tmp[0].get<std::string>();
            int int_port = tmp[1].get<int>();
            if (int_port < 0)
                throw std::invalid_argument("invalid port value");
            std::string port = std::to_string(int_port);
            std::string name = tmp[2].get<std::string>();
            std::string root = tmp[3].get<std::string>();
            serv.VHosts_.push_back(VHostConfig(ip, port, name, root));
        }
        return serv;
    }

    int test_file(const std::string& path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open())
        {
            std::clog << "Provided file invalid: cannot open file\n";
            return 1;
        }

        json j = json::parse(ifs);

        if (j.size() == 0)
        {
            std::clog << "Provided file invalid: empty file\n";
            return 1;
        }

        for (auto i : j)
        {
            json tmp(i);
            if (tmp.size() != 4 && tmp.size() != 5)
            {
                std::clog << "Provided file invalid: wrong configuration\n";
                return 1;
            }
            std::string ip = tmp[0].get<std::string>();
            int int_port = tmp[1].get<int>();
            if (int_port < 0)
            {
                std::clog << "Provided file invalid: invalid port\n";
                return 1;
            }
            std::string name = tmp[2].get<std::string>();
            std::string root = tmp[3].get<std::string>();
        }
        return 0;

    }
    // FIXME
} // namespace http
