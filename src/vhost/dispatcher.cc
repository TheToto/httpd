#include "dispatcher.hh"

#include <iostream>

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
