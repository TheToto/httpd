/**
 * \file config/config.hh
 * \brief Declaration of ServerConfig and VHostConfig.
 */

#pragma once

#include <string>
#include <vector>

namespace http
{
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
                    std::string root, std::string def = "index.html")
            : ip_(ip)
            , port_(port)
            , server_name_(server_name)
            , root_(root)
            , default_file_(def)
        {
            is_ipv6_ = false;
            ipv6_ = "[" + ip_ + "]";
            server_name_port_ = server_name_ + ":" + std::to_string(port_);
            ip_port_ = ip_ + ":" + std::to_string(port_);
            ipv6_port_ = ipv6_ + ":" + std::to_string(port_);
        }

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
