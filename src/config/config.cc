#include "config/config.hh"

#include "error/not-implemented.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Winline"
#ifdef __clang__
#    pragma GCC diagnostic ignored "-Wlogical-op-parentheses"
#endif
#pragma GCC diagnostic pop
#include <fstream> //ifstream
#include <iostream> //clog
#include <json.hpp>
#include <list>
#include <optional>
#include <set>
#include <stdexcept> //invalid_argument
#include <string> //to_string

#include "misc/openssl/base64.hh"

using json = nlohmann::json;

namespace http
{
    ProxyConfig::ProxyConfig(json& proxy)
    {
        ip_ = proxy["ip"];
        port_ = proxy["port"];
        std::string tmp;
        std::string tmp2;
        try
        {
            for (auto i : proxy["proxy_set_header"].items())
            {
                tmp = i.value();
                tmp2 = i.key();
                proxy_set_header[tmp2] = tmp;
            }
        }
        catch (const std::exception&)
        {}
        try
        {
            for (auto i : proxy["proxy_remove_header"])
            {
                tmp = i;
                proxy_remove_header.insert(tmp);
            }
        }
        catch (const std::exception&)
        {}
        try
        {
            for (auto i : proxy["set_header"].items())
            {
                tmp = i.value();
                tmp2 = i.key();
                set_header[tmp2] = tmp;
            }
        }
        catch (const std::exception&)
        {}
        try
        {
            for (auto i : proxy["remove_header"])
            {
                tmp = i;
                remove_header.insert(tmp);
            }
        }
        catch (const std::exception&)
        {}

        ipv6_ = "[" + ip_ + "]";
        ip_port_ = ip_ + ":" + std::to_string(port_);
        ipv6_port_ = ipv6_ + ":" + std::to_string(port_);
    }

    VHostConfig::VHostConfig(std::string ip, int port, std::string server_name,
                             std::string root, std::string def,
                             std::string sslc, std::string sslk,
                             std::optional<ProxyConfig> proxy,
                             std::optional<std::string> authb,
                             std::optional<std::list<std::string>> authbu,
                             std::string health, bool autoi, bool def_vh)
        : ip_(ip)
        , port_(port)
        , server_name_(server_name)
        , root_(root)
        , default_file_(def)
        , ssl_cert_(sslc)
        , ssl_key_(sslk)
        , proxy_pass_(proxy)
        , auth_basic_(authb)
        , auth_basic_users_(authbu)
        , health_endpoint_(health)
        , auto_index_(autoi)
        , default_vhost_(def_vh)
    {
        mode = AF_INET;
        addr.sin_port = port;
        addr6.sin6_port = port;
        addr.sin_family = AF_INET;
        addr6.sin6_family = AF_INET6;
        if (!inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr))
        {
            inet_pton(AF_INET6, ip_.c_str(), &addr6.sin6_addr);
            mode = AF_INET6;
        }
        else
            addr.sin_family = AF_INET;
        addr.sin_port = port_;
        ipv6_ = "[" + ip_ + "]";
        server_name_port_ = server_name_ + ":" + std::to_string(port_);
        ip_port_ = ip_ + ":" + std::to_string(port_);
        ipv6_port_ = ipv6_ + ":" + std::to_string(port_);
    }

    void ServerConfig::vhost_check()
    {
        for (size_t i = 0; i < VHosts_.size(); i++)
        {
            for (size_t j = i + 1; j < VHosts_.size(); j++)
                {
                    if ((VHosts_[i].ip_ == VHosts_[j].ip_
                        || VHosts_[i].ip_ == "0.0.0.0"
                        || VHosts_[j].ip_ == "0.0.0.0")
                        && VHosts_[i].port_ == VHosts_[j].port_
                        && VHosts_[i].server_name_ == VHosts_[j].server_name_)
                            throw std::invalid_argument("VHosts "
                                "must be differenciable");
                }
        }
    }

    static void parse_vhost(nlohmann::basic_json<>& i, ServerConfig& serv)
    {
        std::string ip = i["ip"];
        int port = i["port"];
        std::string server_name = i["server_name"];

        if (ip.empty() || port <= 0 || server_name.empty())
            throw std::invalid_argument("invalid JSON file: a mandatory "
                                        "argument is missing");

        std::string root;
        try
        {
            root = i["root"];
        }
        catch (const std::exception& e)
        {
            root = ".";
        }
        std::string default_file;
        try
        {
            default_file = i["default_file"];
        }
        catch (const std::exception& e)
        {
            default_file = "index.html";
        }
        std::string ssl_cert = "";
        try
        {
            ssl_cert = i["ssl_cert"];
        }
        catch (const std::exception& e)
        {}

        std::string ssl_key = "";
        try
        {
            ssl_key = i["ssl_key"];
        }
        catch (const std::exception& e)
        {}
        if (ssl_key.empty() != ssl_cert.empty())
            throw std::invalid_argument("ERROR: ssl_cert and ssl_key must "
                                        "be defined simultaneously");

        std::optional<std::string> auth_basic = std::nullopt;
        try
        {
            auth_basic = i["auth_basic"];
        }
        catch (const std::exception& e)
        {
            auth_basic = std::nullopt;
        }

        std::optional<std::list<std::string>> auth_basic_users = std::nullopt;
        try
        {
            auto list = i.find("auth_basic_users");
            if (list != i.end())
            {
                auth_basic_users = std::list<std::string>();
                for (std::string cur : i["auth_basic_users"])
                    auth_basic_users.value()
                        .push_front(ssl::base64_encode(cur));
            }
        }
        catch (const std::exception& e)
        {
            auth_basic_users = std::nullopt;
        }

        if (auth_basic_users.has_value() != auth_basic.has_value())
            throw std::invalid_argument("ERROR: auth_basic and auth_basic_users"
                                        " must be defined simultaneously");

        std::string health_endpoint = "";
        try
        {
            health_endpoint = i["health_endpoint"];
        }
        catch (const std::exception& e)
        {}

        bool auto_index = false;
        try
        {
            auto_index = i["auto_index"];
        }
        catch (const std::exception& e)
        {}

        bool default_vhost = false;
        try
        {
            default_vhost = i["default_vhost"];
        }
        catch (const std::exception& e)
        {}

        std::optional<ProxyConfig> proxy = std::nullopt;
        try
        {
            proxy = ProxyConfig(i["proxy_pass"]);
        }
        catch (const std::exception& e)
        {}

        if ((proxy != std::nullopt)
            && (root != "." || default_file != "index.html"
                || auto_index != false || default_vhost != false))
            throw std::invalid_argument(
                "ERROR: a proxy_pass cannot be set if "
                "root/defailt_file/auto_index/default_vhost is set");

        serv.VHosts_.push_back(
            VHostConfig(ip, port, server_name, root, default_file, ssl_cert,
                        ssl_key, proxy, auth_basic, auth_basic_users,
                        health_endpoint, auto_index, default_vhost));
    }

    ServerConfig parse_configuration(const std::string& path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open())
            throw std::invalid_argument("cannot open file\n");
        json j = json::parse(ifs);
        ServerConfig serv;

        try
        {
            serv.payload_max_size = j["payload_max_size"];
        }
        catch (const std::exception&)
        {}
        try
        {
            serv.uri_max_size = j["uri_max_size"];
        }
        catch (const std::exception&)
        {}
        try
        {
            serv.header_max_size = j["header_max_size"];
        }
        catch (const std::exception&)
        {}

        auto ctn = j["vhosts"];

        for (auto i : ctn)
        {
            json tmp(i);
            parse_vhost(tmp, serv);
        }
        serv.vhost_check();
        return serv;
    }

    int test_file(const std::string& path)
    {
        try
        {
            auto res = parse_configuration(path);
        }
        catch (const std::exception& e)
        {
            std::clog << "An error happened while testing:\n"
                      << e.what() << '\n';
            return 1;
        }
        return 0;
    }
} // namespace http
