#include <iostream>
#include "dispatcher.hh"
#include "vhost-factory.hh"
namespace http
{
    shared_vhost Dispatcher::operator()(Request& r)
    {
        const std::string& host = r.get_header("Host");
        auto cur = vhosts_.begin();
        for (; cur != vhosts_.end(); cur++)
        {
            auto conf = (*cur)->conf_get();
            if (conf.server_name_ == host && conf.port_ == 80)
                break;
            if (conf.server_name_port_ == host)
                break;
            if (conf.is_ipv6_)
            {
                if (conf.ipv6_ == host && conf.port_ == 80)
                    break;
                if (conf.ipv6_port_ == host)
                    break;
            }
            if (conf.ip_ == host && conf.port_ == 80)
                break;
            if (conf.ip_port_ == host)
                break;
        }
        if (cur == vhosts_.end())
        {
            if (!r.is_erroring())
                r.set_mode("ERROR");
            std::clog << "No vhost found for this request...\n";
            return VHostFactory::Fail();
        }
        return *cur;
    }
} // namespace http
