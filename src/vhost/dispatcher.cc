#include "dispatcher.hh"

#include <iostream>

#include "vhost-factory.hh"
namespace http
{
    bool testing_ip(int mode, const char* ip)
    {
        sockaddr_in addr;
        if (!inet_pton(mode, ip, &addr.sin_addr))
            return false;
        return true;
    }

    static std::string parse_host(const std::string my_host, sockaddr_in& addr,
                           sockaddr_in6& addr6, int& mode)
    {
        int port = 0;
        std::string ip;
        std::string server_name;

        addr.sin_family = AF_INET;

        auto cur = my_host.find(']');
        if (cur != std::string::npos)
        {
            auto cur_port = my_host.find_last_of(':');
            if (cur_port > cur && cur_port < std::string::npos - 1)
                port = atoi(my_host.substr(cur_port + 1).c_str());
            auto cur_deb_ip = my_host.find('[');
            ip = my_host.substr(cur_deb_ip + 1, cur - cur_deb_ip - 1);
            mode = AF_INET6;
        }
        else
        {
            auto cur_port = my_host.find_last_of(':');
            if (cur_port < std::string::npos - 1)
                port = atoi(my_host.substr(cur_port + 1).c_str());
            ip = my_host.substr(0, cur_port);
            if (testing_ip(AF_INET, ip.c_str()))
                mode = AF_INET;
            else
                server_name = std::string(ip.c_str());
        }
        if (!server_name.length())
        {
            if (mode == AF_INET)
                inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
            else
                inet_pton(AF_INET6, ip.c_str(), &addr6.sin6_addr);
            addr.sin_family = mode;
            addr6.sin6_family = mode;
        }
        addr.sin_port = port;
        addr6.sin6_port = port;
        if (server_name == "")
            return "";
        return server_name;
    }

    static bool compare_addr6(sockaddr_in6 s1, sockaddr_in6 s2)
    {
        return memcmp(s1.sin6_addr.s6_addr, s2.sin6_addr.s6_addr,
                      sizeof(s1.sin6_addr.s6_addr));
    }

    shared_vhost Dispatcher::operator()(Request& r)
    {
        const std::string& host = r.get_header("Host");
        sockaddr_in addr;
        sockaddr_in6 addr6;
        int mode = AF_INET;
        std::string this_server_name = parse_host(host, addr, addr6, mode);
        sockaddr_in addr_def;
        sockaddr_in6 addr6_def;
        shared_vhost default_vhost = nullptr;
        if (mode == AF_INET)
        {
            addr_def.sin_family = AF_INET;
            inet_pton(AF_INET, ROOT_IP, &addr_def.sin_addr);
        }
        else
        {
            addr6_def.sin6_family = AF_INET6;
            inet_pton(AF_INET6, "::", &addr6_def.sin6_addr);
        }
        auto cur = vhosts_.begin();
        for (; cur != vhosts_.end(); cur++)
        {
            auto conf = (*cur)->conf_get();
            if (conf.default_vhost_)
                default_vhost = *cur;
            if (this_server_name != "")
            {
                if (mode == AF_INET)
                {
                    if (!addr.sin_port
                     && conf.server_name_ == this_server_name)
                        break;
                    if (conf.addr.sin_port == addr.sin_port
                     && conf.server_name_ == this_server_name)
                        break;
                }
                else
                {
                    if (!addr6.sin6_port
                     && conf.server_name_ == this_server_name)
                        break;
                    if (conf.addr6.sin6_port == addr6.sin6_port
                     && conf.server_name_ == this_server_name)
                        break;
                }
            }
            else if (conf.mode == mode)
            {
                if (mode == AF_INET)
                {
                    if (!addr.sin_port
                     && (conf.addr.sin_addr.s_addr
                           == addr.sin_addr.s_addr
                     || conf.addr.sin_addr.s_addr
                          == addr_def.sin_addr.s_addr))
                        break;
                    if (conf.addr.sin_port == addr.sin_port
                     && (conf.addr.sin_addr.s_addr
                           == addr.sin_addr.s_addr
                     || conf.addr.sin_addr.s_addr
                          == addr_def.sin_addr.s_addr))
                        break;
                }
                else
                {
                    if (!addr6.sin6_port
                     && (compare_addr6(conf.addr6, addr6)
                     || compare_addr6(conf.addr6, addr6_def)))
                        break;
                    if (conf.addr6.sin6_port == addr6.sin6_port
                     && (compare_addr6(conf.addr6, addr6)
                     ||  compare_addr6(conf.addr6, addr6_def)))
                        break;
                }
            }
        }
        if (cur == vhosts_.end())
        {
            if (default_vhost != nullptr)
                return default_vhost;
            if (!r.is_erroring())
                r.set_mode(MOD::ERROR);
            std::clog << "No vhost found for this request...\n";
            return VHostFactory::Fail();
        }
        return *cur;
    }

    shared_vhost Dispatcher::get_fail() {
        return VHostFactory::Fail();
    }


    const std::vector<shared_vhost> &Dispatcher::getVhosts() const {
        return vhosts_;
    }
} // namespace http
