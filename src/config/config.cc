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
        json j = json::parse(ifs);
        ServerConfig serv;

        for (auto i : j)
        {
            json tmp(i);
            if (tmp.size() != 4)
                throw std::invalid_argument("invalid json file: not a \
json or wrong architecture");
            std::string ip = tmp[0].get<std::string>();
            std::string port = std::to_string(tmp[1].get<int>());
            std::string name = tmp[2].get<std::string>();
            std::string root = tmp[3].get<std::string>();
            serv.VHosts_.push_back(VHostConfig(ip, port, name, root));
        }
        return serv;
    }
    // FIXME
} // namespace http
