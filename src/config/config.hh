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

#define AUTH_BASIC "auth_basic"
#define AUTH_BASIC_USERS "auth_basic_users"
#define AUTO_INDEX "auto_index"
#define CUR_DIR "."
#define DEFAULT_FILE "default_file"
#define DEFAULT_VHOST "default_vhost"
#define FAILOVER "failover"
#define FAILROBIN "fail-robin"
#define HEADER_MS "header_max_size"
#define HEALTH "health"
#define HEALTH_ENDP "health_endpoint"
#define HOSTS "hosts"
#define INDEX "index.html"
#define IP "ip"
#define KEEP_ALIVE "keep_alive"
#define METHOD "method"
#define NB_WORKERS "nb_workers"
#define PAYLOAD_MS "payload_max_size"
#define PORT "port"
#define PROXY_PASS "proxy_pass"
#define PROXY_REMOVE_HEADER "proxy_remove_header"
#define PROXY_SET_HEADER "proxy_set_header"
#define REMOVE_HEADER "remove_header"
#define ROOT "root"
#define ROOT_IP "0.0.0.0"
#define ROUNDROBIN "round-robin"
#define SERVERNAME "server_name"
#define SET_HEADER "set_header"
#define SSL_CERT "ssl_cert"
#define SSL_KEY "ssl_key"
#define TIMEOUT_H "timeout"
#define TRANSACTION "transaction"
#define THP_TIME "throughput_time"
#define THP_VAL "throughput_val"
#define UPSTREAM "upstream"
#define UPSTREAMS "upstreams"
#define URI_MS "uri_max_size"
#define VHOSTS "vhosts"
#define WEIGHT "weight"

using json = nlohmann::json;

namespace http {



    /**
     *  \struct Upstream
     *  \brief Adress of one instance of the Reverse Proxy
     *
     **/

    enum methods{
        failover,
        fail_robin,
        round_robin,
        method_none,
        method_error
    };
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

    static std::string nullStr = "";
    static Upstream nullUpstream(nullStr, -1, -1, nullStr);

    struct upQueue{
        upQueue() = default;
        upQueue(std::vector<Upstream>& v, methods& method);
        Upstream getNext();

        std::string build_health(std::string& heal_ep, std::string& ipPort);
        void set_health(int health, bool is_ok);

        void fillQueue();

        methods m;
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
                    methods& method, std::optional<int>& Timeout);
        ProxyConfig(const ProxyConfig&) = default;
        ProxyConfig& operator=(const ProxyConfig&) = default;
        ProxyConfig(ProxyConfig&&) = default;
        ProxyConfig& operator=(ProxyConfig&&) = default;

        static ProxyConfig parse_upstream(json& proxy, json& j);

        upQueue upstreams;/// Vector of upstreams available for this proxy
        methods method_;    ///what is the current method (RR, failR etc.)."" being no method
        std::map<std::string,std::string> proxy_set_header;
        std::set<std::string> proxy_remove_header;
        std::map<std::string, std::string> set_header;
        std::set<std::string> remove_header;
        std::optional<int> to_;
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

        std::optional<size_t> keep_alive = std::nullopt;
        std::optional<size_t> transaction = std::nullopt;
        std::optional<size_t> throughput_val = std::nullopt;
        std::optional<size_t> throughput_time = std::nullopt;

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
