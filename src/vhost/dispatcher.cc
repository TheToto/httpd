#include "dispatcher.hh"
namespace http
{
    shared_vhost Dispatcher::operator()(Request& r)
    {
        const std::string& host = r.get_header("Host");
        auto cur = vhosts_.begin();
        for(; cur != vhosts_.end(); cur++)
        {
            auto conf = (*cur)->conf_get();
            if (conf.ip_ == host)
                break;
            else if (conf.server_name_ == host)
                break;
            else if (conf.ip_port_ == host)
                break;
            else if (conf.server_name_port_ == host)
                break;
        }
        if (cur == vhosts_.end())
        {
            r.set_mode("ERROR");
            return null_ptr;
        }
        return *cur;
    }
}
