/**
 * \file config/config.hh
 * \brief Declaration of ServerConfig and VHostConfig.
 */

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <map>
#include <list>

namespace http
{


    struct ProxyConfig
    {
        ProxyConfig(nlohmann::basic_json proxy);
        bool is_ipv6_ = false;
        const std::string ip_;
        const int port_;
        std::string ipv6_;
        std::string ip_port_;
        std::string ipv6_port_;

        std::map<std::string> proxy_set_header;
        std::map<std::string> proxy_remove_header;
        std::map<std::string> set_header;
        std::map<std::string> remove_header;
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
                    std::string authb, std::list<std::string> authbu,
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

        std::string ssl_cert_ = "";
        std::string ssl_key_  = "";
        std::optional<ProxyConfig> proxy_pass  = nullopt_t;
        std::string auth_basic  = "";
        std::list<std::string> auth_basic_users ;
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

        std::vector<VHostConfig> VHosts_;
        std::optional<size_t> payload_max_size = std::nullopt_t;
        std::optional<size_t> uri_max_size = std::nullopt_t;
        std::optional<size_t> header_max_size = std::nullopt_t;
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
} // namespace http
