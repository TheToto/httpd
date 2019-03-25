#include "config/config.hh"

#include "error/not-implemented.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Winline"
#ifdef __clang__
#    pragma GCC diagnostic ignored "-Wlogical-op-parentheses"
#endif
#include <fstream> //ifstream
#include <iostream> //clog
#include <json.hpp>
#include <stdexcept> //invalid_argument
#include <string> //to_string
#include <list>
#include <set>
#include <optional>
#pragma GCC diagnostic pop

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
                proxy_set_header[tmp2] =  tmp;
            }
        } catch(const std::exception&) {}
        try
        {
            for (auto i : proxy["proxy_remove_header"])
            {
                tmp = i;
                proxy_remove_header.insert(tmp);
            }
        } catch(const std::exception&) {}
        try
        {
            for (auto i : proxy["set_header"].items())
            {
                tmp = i.value();
                tmp2 = i.key();
                set_header[tmp2] = tmp;
            }
        } catch(const std::exception&) {}
        try
        {
            for (auto i : proxy["remove_header"])
            {
                tmp = i;
                remove_header.insert(tmp);
            }

        } catch(const std::exception&) {}

        ipv6_ = "[" + ip_ + "]";
        ip_port_ = ip_ + ":" + std::to_string(port_);
        ipv6_port_ = ipv6_ + ":" + std::to_string(port_);
    }

    VHostConfig::VHostConfig(std::string ip, int port, std::string server_name,
            std::string root, std::string def, std::string sslc,
            std::string sslk, std::optional<ProxyConfig> proxy,
            std::string authb, std::list<std::string> authbu,
            std::string health, bool autoi, bool def_vh):
        ip_(ip), port_(port), server_name_(server_name), root_(root),
        default_file_(def), ssl_cert_(sslc), ssl_key_(sslk), proxy_pass_(proxy),
        auth_basic_(authb), auth_basic_users_(authbu), health_endpoint_(health),
        auto_index_(autoi), default_vhost_(def_vh)
    {
        if (!inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr))
            inet_pton(AF_INET6, ip_.c_str(), &addr.sin_addr);
        
        ipv6_ = "[" + ip_ + "]";
        server_name_port_ = server_name_ + ":" + std::to_string(port_);
        ip_port_ = ip_ + ":" + std::to_string(port_);
        ipv6_port_ = ipv6_ + ":" + std::to_string(port_);
    }

    static void parse_vhost(nlohmann::basic_json<>& i, ServerConfig& serv)
    {
        std::string ip = i["ip"];
        int port = i["port"];
        std::string server_name = i["server_name"];

        if (ip.empty() || port <= 0 || server_name.empty())
            throw std::invalid_argument("invalid JSON file: a mandatory \
argument is missing");

        std::string root;
        try
        {
            root = i["root"];
        } catch (const std::exception& e){
            root = ".";
        }
        std::string default_file;
        try
        {
            default_file = i["default_file"];
        } catch (const std::exception& e){
            default_file = "index.html";
        }
        std::string ssl_cert = "";
        try
        {
            ssl_cert = i["ssl_cert"];
        } catch (const std::exception& e){}

        std::string ssl_key = "";
        try
        {
            ssl_key = i["ssl_key"];
        } catch (const std::exception& e){}
        if (ssl_key.empty() != ssl_cert.empty())
            throw std::invalid_argument("ERROR: ssl_cert and ssl_key must \
be defined simulteanously");

        std::string auth_basic = "";
        try
        {
            auth_basic = i["auth_basic"];
        } catch (const std::exception& e){}

        std::list<std::string> auth_basic_users;
        try
        {
            for (std::string cur : json(i["auth_basic_users"]))
                auth_basic_users.push_front(cur);
        } catch (const std::exception& e){}

        if (auth_basic_users.empty() != auth_basic.empty())
            throw std::invalid_argument("ERROR: auth_basic and auth_basic_users\
 must be defined simulteanously");

        std::string health_endpoint = "";
        try
        {
            health_endpoint = i["auth_endpoint"];
        } catch (const std::exception& e){}

        bool auto_index = false;
        try
        {
             auto_index = i["auto_index"];
        } catch (const std::exception& e){}

        bool default_vhost = false;
        try
        {
             default_vhost = i["default_vhost"];
        } catch (const std::exception& e){}


        std::optional<ProxyConfig> proxy = std::nullopt;
        try
        {
            proxy = ProxyConfig(i["proxy_pass"]);

        } catch (const std::exception& e){}

        if ((proxy != std::nullopt) && (root != "." || default_file != "index.html"
                    || auto_index != false || default_vhost != false))
            throw std::invalid_argument("ERROR: a proxy_pass cannot be set if \
root/defailt_file/auto_index/default_vhost is set");

        serv.VHosts_.push_back(VHostConfig(ip, port, server_name, root,
                default_file, ssl_cert, ssl_key, proxy, auth_basic,
                auth_basic_users, health_endpoint, auto_index, default_vhost));
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
        } catch(const std::exception&){}
        try
        {
            serv.uri_max_size = j["uri_max_size"];
        } catch(const std::exception&){}
        try
        {
            serv.header_max_size = j["header_max_size"];
        } catch(const std::exception&){}

        auto ctn = j["vhosts"];

        for (auto i : ctn)
        {
            json tmp(i);
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
        ServerConfig serv;

        auto ctn = j["vhosts"];

        for (auto i : ctn)
        {
            json tmp(i);
            if (tmp.size() != 4 && tmp.size() != 5)
            {
                std::clog << ("invalid json file: not a \
                        json or wrong architecture\n");
                return 1;
            }
            std::string ip = tmp["ip"];
            int port = tmp["port"];
            std::string server_name = tmp["server_name"];
            std::string root = tmp["root"];
            std::string default_file;
            try
            {
                default_file = tmp["default_file"];
            }
            catch (const std::exception& e)
            {
                default_file = "index.html";
            }

            if (ip.empty() || port <= 0 || server_name.empty() || root.empty())
            {
                std::clog << ("invalid JSON file: a mandatory \
                        argument is missing\n");
                return 1;
            }
        }

        return 0;
    }
} // namespace http
