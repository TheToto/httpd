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

    std::string parse_host(const std::string my_host, sockaddr_in& addr)
    {
        int port = 0;
        int mode = -1;
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
            inet_pton(mode, ip.c_str(), &addr.sin_addr);
            addr.sin_family = mode;
        }
        addr.sin_port = port;
        if (server_name == "")
            return "";
        return server_name;
    }

    shared_vhost Dispatcher::operator()(Request& r)
    {
        const std::string& host = r.get_header("Host");
        sockaddr_in addr;
        std::string this_server_name = parse_host(host, addr);
        sockaddr_in addr_def;
        if (addr.sin_family == AF_INET)
        {
            addr_def.sin_family = AF_INET;
            inet_pton(AF_INET, "0.0.0.0", &addr_def.sin_addr);
        }
        else
        {
            addr_def.sin_family = AF_INET6;
            inet_pton(AF_INET6, "::", &addr_def.sin_addr);
        }
        auto cur = vhosts_.begin();
        for (; cur != vhosts_.end(); cur++)
        {
            auto conf = (*cur)->conf_get();
            if (this_server_name != "")
            {
                if (!addr.sin_port && conf.server_name_ == this_server_name)
                    break;
                if (conf.addr.sin_port == addr.sin_port
                 && conf.server_name_ == this_server_name)
                    break;
            }
            else if (conf.addr.sin_family == addr.sin_family)
            {
                if (!addr.sin_port
                 && (conf.addr.sin_addr.s_addr == addr.sin_addr.s_addr
                 || conf.addr.sin_addr.s_addr == addr_def.sin_addr.s_addr))
                    break;
                if (conf.addr.sin_port == addr.sin_port
                 && (conf.addr.sin_addr.s_addr == addr.sin_addr.s_addr
                 || conf.addr.sin_addr.s_addr == addr_def.sin_addr.s_addr))
                    break;
            }
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
