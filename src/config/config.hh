/**
 * \file config/config.hh
 * \brief Declaration of ServerConfig and VHostConfig.
 */

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <set>
#include <list>
#include <json.hpp>
#include <arpa/inet.h>
#include <queue>

using json = nlohmann::json;

namespace http
{

    /**
     *  \struct Upstream
     *  \brief Adress of one instance of the Reverse Proxy
     *
     **/
    struct Upstream{

        Upstream(std::string& ip, int port, int weight, std::string& health);
        Upstream() = default;
        bool isNull();

        std::string ip_;
        int port_;
        std::string ipv6_;
        std::string ip_port_;
        std::string ipv6_port_;
        std::string health_; /// Usefull if method checks failed/dead proxy, "" meaning method does not
        bool alive = true;
        int weight_;        /// Do not use
    };

    ///static ref for performance issues
    static Upstream nullUpstream((std::string &) "", -1, 0, (std::string &) "");

    struct upQueue{
        upQueue() = default;
        upQueue(std::vector<Upstream>& v);
        Upstream getNext();

        private:
            void fillQueue();
            std::deque<int> queue;
            std::vector<Upstream> vector;
    };


    /**
     *  \struct ProxyConfig
     *  \brief Contains all informations about the reverse proxy whose attach to the VHost
     *
     **/
    struct ProxyConfig
    {
        ProxyConfig(json& proxy, std::vector<Upstream> v,
                    std::string& method);
        ProxyConfig(const ProxyConfig&) = default;
        ProxyConfig& operator=(const ProxyConfig&) = default;
        ProxyConfig(ProxyConfig&&) = default;
        ProxyConfig& operator=(ProxyConfig&&) = default;

        static ProxyConfig parse_upstream(json& proxy, json& j);

        upQueue upstreams;/// Vector of upstreams available for this proxy
        std::string method_;    ///what is the current method (RR, failR etc.)."" being no method
        std::map<std::string,std::string> proxy_set_header;
        std::set<std::string> proxy_remove_header;
        std::map<std::string, std::string> set_header;
        std::set<std::string> remove_header;
    };

    /**
     * \struct VHostConfig
     * \brief Value object storing a virtual host configuration.
     *
     * Since each virtual host of the server has its own configuration, a
     * dedicated structure is required to store the information related to
     * each one of them.
     */
    struct VHostConfig
    {
        VHostConfig()
            : ip_("")
            , port_(0)
            , server_name_("")
            , root_("")
        {}

        VHostConfig(std::string ip, int port, std::string server_name,
                    std::string root, std::string def, std::string sslc,
                    std::string sslk, std::optional<ProxyConfig> proxy,
                    std::optional<std::string> authb,
                    std::optional<std::list<std::string>> authbu,
                    std::string health, bool autoi, bool def_vh);

        VHostConfig(const VHostConfig&) = default;
        VHostConfig& operator=(const VHostConfig&) = default;
        VHostConfig(VHostConfig&&) = default;
        VHostConfig& operator=(VHostConfig&&) = default;

        ~VHostConfig() = default;

        const std::string ip_;
        const int port_;
        const std::string server_name_;

        bool is_ipv6_ = false;
        std::string ipv6_;
        std::string server_name_port_;
        std::string ip_port_;
        std::string ipv6_port_;

        const std::string root_;
        const std::string default_file_ = "index.html";

        sockaddr_in addr;
        sockaddr_in6 addr6;
        int mode = AF_INET;
        std::string ssl_cert_ = "";
        std::string ssl_key_  = "";

        std::optional<ProxyConfig> proxy_pass_  = std::nullopt;

        std::optional<std::string> auth_basic_  = std::nullopt;
        std::optional<std::list<std::string>> auth_basic_users_ = std::nullopt;
        std::string health_endpoint_  = "";
        bool auto_index_  = false;
        bool default_vhost_  = false;
    };

    /**
     * \struct ServerConfig
     * \brief Value object storing the server configuration.
     *
     * To avoid opening the configuration file each time we need to access the
     * server configuration, a dedicated structure is required to store it.
     */
    struct ServerConfig
    {
        ServerConfig() = default;
        ServerConfig(const ServerConfig&) = default;
        ServerConfig& operator=(const ServerConfig&) = default;
        ServerConfig(ServerConfig&&) = default;
        ServerConfig& operator=(ServerConfig&&) = default;

        ~ServerConfig() = default;

        void vhost_check();

        std::vector<VHostConfig> VHosts_;
        std::optional<size_t> payload_max_size = std::nullopt;
        std::optional<size_t> uri_max_size = std::nullopt;
        std::optional<size_t> header_max_size = std::nullopt;
        int nb_workers = 1;
        bool is_default = false;
    };

    /**
     * \brief Parse the server configuration file.
     *
     * \param path string containing the path to the server configuration
     * file.
     * \return The server configuration.
     */
    struct ServerConfig parse_configuration(const std::string& path);
    int test_file(const std::string& path);
    extern ServerConfig serv_conf;

} // namespace http
