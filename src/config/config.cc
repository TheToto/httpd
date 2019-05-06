#include <random>

#include "config/config.hh"

#include "error/not-implemented.hh"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Winline"
#ifdef __clang__
#    pragma GCC diagnostic ignored "-Wlogical-op-parentheses"
#endif
#pragma GCC diagnostic pop
#include <fstream> //ifstream
#include <iostream> //clog
#include <json.hpp>
#include <list>
#include <optional>
#include <set>
#include <stdexcept> //invalid_argument
#include <string> //to_string
#include <queue>

#include "request/types.hh"

#include "misc/openssl/base64.hh"

using json = nlohmann::json;

namespace http
{

    Upstream::Upstream(std::string& ip, int port, int weight, std::string& health, int index):
        ip_(ip), port_(port), health_(health), index_(index), weight_(weight)
    {
        ipv6_ = "[" + ip_ + "]";
        ip_port_ = ip_ + ":" + std::to_string(port_);
        ipv6_port_ = ipv6_ + ":" + std::to_string(port_);
    }

    bool Upstream::isNull() {
        return port_ == -1;
    }

    static methods whichMethod(std::string& str){
        if (str.empty())
            return method_none;
        if (str == ROUNDROBIN)
            return round_robin;
        if (str == FAILOVER)
            return failover;
        if (str == FAILROBIN)
            return fail_robin;
        return method_error;
    }

    ProxyConfig ProxyConfig::parse_upstream(json& proxy,
            json& j){

        std::vector<Upstream> v;
        std::string upstreamLink = "";
        std::string ip = "";
        std::string method = "";
        methods m = method_none;
        int weight = 1;
        int port = 0;
        std::optional<int> Timeout;
        std::string health = "";

        try {
            upstreamLink = proxy[UPSTREAM];
        }
        catch (const std::exception&) {}
        try {
            ip = proxy[IP];
            port = proxy[PORT];
        }
        catch (std::exception& e){
            if (upstreamLink.empty())
                throw e;
        }

        try {
            Timeout = proxy[TIMEOUT_H];
        }
        catch (const std::exception&){}

        if (!ip.empty()){
            v.push_back(Upstream(ip, port, 2, health, v.size()));
            return ProxyConfig(proxy, v, m, Timeout);
        }
        method = j[UPSTREAMS][upstreamLink][METHOD];
        m = whichMethod(method);
        if (m == method_error)
            throw std::invalid_argument("Method used in upstreams not recognized: " + method);
        for (auto i: j[UPSTREAMS][upstreamLink][HOSTS]){
            ip = i[IP];
            port = i[PORT];
            try {
                weight = i[WEIGHT];
            }
            catch (std::exception&){
                weight = 1;
            }
            if (method == FAILOVER || method == FAILROBIN) {
                health = i[HEALTH];
                if (health.empty())
                    throw std::invalid_argument("Health field must be precised in each upstream def");
            }
            v.push_back(Upstream(ip, port, weight, health, v.size()));
            health = "";
        }
        return ProxyConfig(proxy, v, m,Timeout);
    }

    ProxyConfig::ProxyConfig(json& proxy, std::vector<Upstream> v,
            methods& method, std::optional<int>& Timeout)
    {
        std::string tmp;
        std::string tmp2;
        try
        {
            for (auto i : proxy[PROXY_SET_HEADER].items())
            {
                tmp = i.value();
                tmp2 = i.key();
                proxy_set_header[tmp2] = tmp;
            }
        }
        catch (const std::exception&)
        {}
        try
        {
            for (auto i : proxy[PROXY_REMOVE_HEADER])
            {
                tmp = i;
                proxy_remove_header.insert(tmp);
            }
        }
        catch (const std::exception&)
        {}
        try
        {
            for (auto i : proxy[SET_HEADER].items())
            {
                tmp = i.value();
                tmp2 = i.key();
                set_header[tmp2] = tmp;
            }
        }
        catch (const std::exception&)
        {}
        try
        {
            for (auto i : proxy[REMOVE_HEADER])
            {
                tmp = i;
                remove_header.insert(tmp);
            }
        }
        catch (const std::exception&)
        {}

        method_ = method;
        upstreams = upQueue(v, method);
        to_ = Timeout;
    }

    VHostConfig::VHostConfig(std::string ip, int port, std::string server_name,
                             std::string root, std::string def,
                             std::string sslc, std::string sslk,
                             std::optional<ProxyConfig> proxy,
                             std::optional<std::string> authb,
                             std::optional<std::list<std::string>> authbu,
                             std::string health, std::string old_permanent,
                             bool autoi, bool def_vh)
        : ip_(ip)
        , port_(port)
        , server_name_(server_name)
        , root_(root)
        , default_file_(def)
        , ssl_cert_(sslc)
        , ssl_key_(sslk)
        , proxy_pass_(proxy)
        , auth_basic_(authb)
        , auth_basic_users_(authbu)
        , health_endpoint_(health)
        , old_uri_perm(old_permanent)
        , auto_index_(autoi)
        , default_vhost_(def_vh)
    {
        mode = AF_INET;
        addr.sin_port = port;
        addr6.sin6_port = port;
        addr.sin_family = AF_INET;
        addr6.sin6_family = AF_INET6;
        if (!inet_pton(AF_INET, ip_.c_str(), &addr.sin_addr))
        {
            inet_pton(AF_INET6, ip_.c_str(), &addr6.sin6_addr);
            mode = AF_INET6;
        }
        else
            addr.sin_family = AF_INET;
        addr.sin_port = port_;
        ipv6_ = "[" + ip_ + "]";
        server_name_port_ = server_name_ + ":" + std::to_string(port_);
        ip_port_ = ip_ + ":" + std::to_string(port_);
        ipv6_port_ = ipv6_ + ":" + std::to_string(port_);
    }

    void ServerConfig::vhost_check()
    {
        for (size_t i = 0; i < VHosts_.size(); i++)
        {
            for (size_t j = i + 1; j < VHosts_.size(); j++)
                {
                    if ((VHosts_[i].ip_ == VHosts_[j].ip_
                        || VHosts_[i].ip_ == ROOT_IP
                        || VHosts_[j].ip_ == ROOT_IP)
                        && VHosts_[i].port_ == VHosts_[j].port_
                        && VHosts_[i].server_name_ == VHosts_[j].server_name_)
                            throw std::invalid_argument("VHosts "
                                "must be differenciable");
                }
        }
    }

    static void parse_vhost(nlohmann::basic_json<>& i, ServerConfig& serv,
            nlohmann::basic_json<>& j)
    {
        std::string ip = i[IP];
        int port = i[PORT];
        std::string server_name = i[SERVERNAME];

        if (ip.empty() || port <= 0 || server_name.empty())
            throw std::invalid_argument("invalid JSON file: a mandatory "
                                        "argument is missing");

        std::string root;
        try
        {
            root = i[ROOT];
        }
        catch (const std::exception& e)
        {
            root = CUR_DIR;
        }
        std::string default_file;
        try
        {
            default_file = i[DEFAULT_FILE];
        }
        catch (const std::exception& e)
        {
            default_file = INDEX;
        }
        std::string ssl_cert = "";
        try
        {
            ssl_cert = i[SSL_CERT];
        }
        catch (const std::exception& e)
        {}

        std::string ssl_key = "";
        try
        {
            ssl_key = i[SSL_KEY];
        }
        catch (const std::exception& e)
        {}
        if (ssl_key.empty() != ssl_cert.empty())
            throw std::invalid_argument("ERROR: ssl_cert and ssl_key must "
                                        "be defined simultaneously");

        std::optional<std::string> auth_basic = std::nullopt;
        try
        {
            auth_basic = i[AUTH_BASIC];
        }
        catch (const std::exception& e)
        {
            auth_basic = std::nullopt;
        }

        std::optional<std::list<std::string>> auth_basic_users = std::nullopt;
        try
        {
            auto list = i.find(AUTH_BASIC_USERS);
            if (list != i.end())
            {
                auth_basic_users = std::list<std::string>();
                for (std::string cur : i[AUTH_BASIC_USERS])
                    auth_basic_users.value()
                        .push_front(ssl::base64_encode(cur));
            }
        }
        catch (const std::exception& e)
        {
            auth_basic_users = std::nullopt;
        }

        if (auth_basic_users.has_value() != auth_basic.has_value())
            throw std::invalid_argument("ERROR: auth_basic and auth_basic_users"
                                        " must be defined simultaneously");

        std::string health_endpoint = "";
        try
        {
            health_endpoint = i[HEALTH_ENDP];
        }
        catch (const std::exception& e)
        {}

        std::string old_permanent = "";
        try
        {
            old_permanent = i[OLD_PERM];
        }
        catch (const std::exception& e)
        {}

        bool auto_index = false;
        try
        {
            auto_index = i[AUTO_INDEX];
        }
        catch (const std::exception& e)
        {}

        bool default_vhost = false;
        try
        {
            default_vhost = i[DEFAULT_VHOST];
        }
        catch (const std::exception& e)
        {}

        std::optional<ProxyConfig> proxy = std::nullopt;
        try
        {
            proxy = ProxyConfig::parse_upstream(i[PROXY_PASS], j);
        }
        catch (const std::exception& e)
        {}

        if ((proxy != std::nullopt)
            && (root != CUR_DIR || default_file != INDEX
                || auto_index != false || default_vhost != false))
            throw std::invalid_argument(
                "ERROR: a proxy_pass cannot be set if "
                "root/default_file/auto_index/default_vhost is set");

        serv.VHosts_.push_back(
            VHostConfig(ip, port, server_name, root, default_file, ssl_cert,
                        ssl_key, proxy, auth_basic, auth_basic_users,
                        health_endpoint, old_permanent,
                        auto_index, default_vhost));
    }

    ServerConfig parse_configuration(const std::string& path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open())
            throw std::invalid_argument("cannot open file\n");
        json j = json::parse(ifs);
        ServerConfig serv;

        try
        {
            serv.payload_max_size = j[PAYLOAD_MS];
        }
        catch (const std::exception&)
        {}
        try
        {
            serv.uri_max_size = j[URI_MS];
        }
        catch (const std::exception&)
        {}
        try
        {
            serv.header_max_size = j[HEADER_MS];
        }
        catch (const std::exception&)
        {}

        json TO;
        try {
            TO = j[TIMEOUT_H];
        } catch (const std::exception&){}

        if (TO.size() > 0) {
            try {
                serv.keep_alive = TO[KEEP_ALIVE];
            }
            catch (const std::exception &e) {}
            try {
                serv.transaction = TO[TRANSACTION];
            }
            catch (const std::exception &) {}
            try {
                serv.throughput_val = TO[THP_VAL];
                serv.throughput_time = TO[THP_TIME];
            }
            catch (const std::exception &) {}

            if (serv.throughput_val.has_value() != serv.throughput_time.has_value())
                throw std::invalid_argument("Both throughput's value and "
                                            "time have to be set");//TODO test if this case works
        }
        try{
            serv.nb_workers = j[NB_WORKERS];
        }
        catch (const std::exception&)
        {
            serv.nb_workers = 1;
        }
        if (serv.nb_workers < 0)
            throw std::invalid_argument("Number of workers "
                                            "cannot be negative");

        auto ctn = j[VHOSTS];

        for (auto i : ctn)
        {
            json tmp(i);
            parse_vhost(tmp, serv, j);
        }
        serv.vhost_check();
        return serv;
    }

    int test_file(const std::string& path)
    {
        try
        {
            auto res = parse_configuration(path);
        }
        catch (const std::exception& e)
        {
            std::clog << "An error happened while testing:\n"
                      << e.what() << '\n';
            return 1;
        }
        return 0;
    }

    upQueue::upQueue(std::vector<Upstream> &v, methods& method) {
        m = method;
        vector = v;
        fillQueue();
    }

    Upstream upQueue::getNext() {
        if (queue.empty())
        {
            fillQueue();
        }
        Upstream res;
        do {
            if (queue.empty())
                return nullUpstream;
            res = vector[queue.front()];
            if (m != failover || !res.alive)
                queue.pop_front();
        } while ((m == failover || m == fail_robin) && !res.alive);
        return res;
    }

    void upQueue::fillQueue() {
        std::deque<int> q;
        for (unsigned long index2 = 0; index2 < vector.size(); index2++){
            if (vector[index2].alive) {
                for (int index = 0; index < vector[index2].weight_; index++)
                    q.push_back(index2);
            }
        }
        std::shuffle(q.begin(), q.end(), std::mt19937(std::random_device()()));
        queue = q;
    }

    void upQueue::set_health(int health, bool is_ok) {
        vector[health].alive = is_ok;
    }

    std::string upQueue::build_health(std::string &heal_ep, std::string &ipPort){
        return "GET " + heal_ep + " HTTP/1.1" + http_crlf + "Host: " + ipPort + http_crlfx2;
    }
} // namespace http
