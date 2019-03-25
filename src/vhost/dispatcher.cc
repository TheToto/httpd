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

    void parse_host(const std::string my_host, sockaddr_in addr, std::string& server_name)
    {
        int port = -1;
        int mode = -1;
        std::string ip;
        auto cur = my_host.find(']');
        if (cur != std::string::npos)
        {
            auto cur_port = my_host.find_last_of(':');
            if (cur_port > cur && cur_port < std::string::npos - 1)
                port = atoi(my_host.substr(cur_port + 1).c_str());
            auto cur_deb_ip = my_host.find('[');
            ip = my_host.substr(cur_deb_ip + 1, cur - cur_deb_ip - 1);
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
                server_name = ip;
        }
        if (server_name == "")
            inet_pton(mode, ip.c_str(), &addr.sin_addr);
        if (port >= 0)
            addr.sin_port = port;
    }

    shared_vhost Dispatcher::operator()(Request& r)
    {
        const std::string& host = r.get_header("Host");
        sockaddr_in addr;
        std::string server_name = "";
        parse_host(host, addr, server_name);

        auto cur = vhosts_.begin();
        for (; cur != vhosts_.end(); cur++)
        {
            auto conf = (*cur)->conf_get();
            if (conf.server_name_ == host)
                break;
            if (conf.server_name_port_ == host)
                break;
            if (conf.ipv6_ == host)
                break;
            if (conf.ipv6_port_ == host)
                break;
            if (conf.ip_port_ == host)
                break;
            if (conf.ip_ == host)
                break;
        }
        if (cur == vhosts_.end())
        {
            if (!r.is_erroring())
                r.set_mode(MOD::ERROR);
            std::clog << "No vhost found for this request...\n";
            return VHostFactory::Fail();
        }
        return *cur;
    }
} // namespace http
