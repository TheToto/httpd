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
    static void parse_vhost(nlohmann::basic_json<>& i, ServerConfig& serv)
    {
        std::string ip = i["ip"];
        int port = i["port"];
        std::string server_name = i["server_name"];
        std::string root = i["root"];
        std::string default_file;
        try
        {
            default_file = i["default_file"];
        }
        catch (const std::exception& e)
        {
            default_file = "index.html";
        }

        if (ip.empty() || port <= 0 || server_name.empty() || root.empty())
            throw std::invalid_argument("invalid JSON file: a mandatory \
argument is missing");
        if (i.size() == 5)
        {
            serv.VHosts_.push_back(VHostConfig(ip, std::to_string(port),
                server_name, root, default_file));
        }
        else
            serv.VHosts_.push_back(VHostConfig(ip, std::to_string(port),
                server_name, root));
    }

    ServerConfig parse_configuration(const std::string& path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open())
            throw std::invalid_argument("cannot open file\n");
        json j = json::parse(ifs);
        ServerConfig serv;

        auto ctn = j["vhosts"];

        for (auto i : ctn)
        {
            json tmp(i);
            if (tmp.size() != 4 && tmp.size() != 5)
                throw std::invalid_argument("invalid json file: not a \
json or wrong architecture");
            parse_vhost(tmp, serv);
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
            std::string test = tmp[0].get<std::string>();
            int int_port = tmp[1].get<int>();
            if (int_port < 0)
            {
                std::clog << "Provided file invalid: invalid port\n";
                return 1;
            }
            test = tmp[2].get<std::string>();
            test = tmp[3].get<std::string>();
            if (tmp.size() == 5)
                test = tmp[4].get<std::string>();
        }
        return 0;

    }
    // FIXME
} // namespace http
