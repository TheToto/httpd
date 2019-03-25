#include "request/request.hh"

#include <iostream>
#include <sstream>

namespace http
{
    int Request::get_mode_str(const std::string& asked, size_t& cur)
    {
        size_t len = asked.size();
        if (len >= 4 && asked.substr(0, 4) == "GET ")
        {
            set_mode(MOD::GET);
            cur = 3;
        }
        else if (len >= 5 && asked.substr(0, 5) == "POST ")
        {
            set_mode(MOD::POST);
            cur = 4;
        }
        else if (len >= 5 && asked.substr(0, 5) == "HEAD ")
        {
            set_mode(MOD::HEAD);
            is_head_ = true;
            cur = 4;
        }
        else
        {
            set_mode(MOD::ERROR_METHOD, 1);
            cur = -1;
        }
        return cur;
    }

    char Request::get_http_version(const std::string& asked, size_t& cur)
    {
        auto pos2 = asked.find("HTTP", cur);
        if (pos2 == std::string::npos)
        {
            set_mode(MOD::ERROR, 1);
            return 0;
        }

        auto p_version = asked.substr(pos2, pos2 + 8);
        if (p_version == "HTTP/1.1")
            return 1;

        if ((p_version[5] <= '0' && p_version[5] > '9')
            || (p_version[5] <= '0' && p_version[7] > '9'))
        {
            set_mode(MOD::ERROR, 1);
            return 0;
        }

        if (p_version[5] == '0' || p_version[7] == '0')
        {
            set_mode(MOD::UPGRADE, 1);
            return 0;
        }

        if (p_version[5] > '1' || p_version[7] > '1')
        {
            set_mode(MOD::OBSOLETE, 1);
            return 0;
        }

        return 1;
    }

    void Request::format_header_val(std::string& h_val)
    {
        int cur_g = 0;
        while (true)
        {
            int beg_sp = h_val.find_first_of(' ', cur_g);
            if (beg_sp < 0)
                return;
            int end_sp = h_val.find_first_not_of(' ', beg_sp);
            if (end_sp < 0)
            {
                h_val.erase(beg_sp);
                return;
            }
            h_val.erase(beg_sp + 1, end_sp - beg_sp - 1);
            cur_g = h_val.find_first_not_of(' ', beg_sp);
        }
    }

    int Request::get_headers_str(const std::string& asked, size_t& cur)
    {
        bool found = 0;
        while (cur > 0)
        {
            cur = asked.find_first_of('\n', cur) + 1;
            if (cur == 0 || asked.substr(cur, 1) == "\r")
                break;
            int len_cur = asked.find_first_of(':', cur) - cur;
            if (len_cur == 4 && asked.substr(cur, 6) == "Host: ")
                found = 1;
            int beg_v = asked.find_first_not_of(' ', cur + len_cur + 1);
            int len_v = asked.find_first_of('\r', beg_v) - beg_v;
            std::string name = asked.substr(cur, len_cur);
            std::string val = "";
            if (len_v > 0)
                val = asked.substr(beg_v, len_v);
            else
                val = asked.substr(beg_v);
            format_header_val(val);
            set_header(name, val);
        }
        if (!found)
        {
            set_mode(MOD::ERROR, 1);
            return -1;
        }
        return cur;
    }

    bool Request::decode_uri(std::string& str)
    {
        size_t i = str.find_first_of('%', 0);
        while (i != std::string::npos)
        {
            if (i + 2 < str.size())
            {
                auto hex = str.substr(i + 1, 2);

                try
                {
                    std::string x(1, std::stol(hex, nullptr, 16));
                    str.replace(i, 3, x);
                    i = str.find_first_of('%', i + 1);
                }
                catch (std::invalid_argument&)
                {
                    set_mode(MOD::ERROR, 1);
                    return false;
                }
            }
            else
            {
                set_mode(MOD::ERROR, 1);
                return false;
            }
        }
        return true;
    }

    bool Request::parse_uri(std::string full_uri)
    {
        int begin_uri = 0;
        int mid_src = full_uri.find("://");
        if (mid_src >= 0)
        {
            begin_uri = full_uri.find_first_of('/', mid_src + 3);
            src = full_uri.substr(0, begin_uri);
        }
        int begin_query = full_uri.find_first_of('?');
        if (begin_query < 0)
            uri = full_uri.substr(begin_uri);
        else
        {
            uri = full_uri.substr(begin_uri, begin_query - begin_uri);
            query = full_uri.substr(begin_query);
        }
        return decode_uri(uri) && decode_uri(query);
    }

    char Request::check_length()
    {
        auto len_str = get_header("Content-Length");
        if (len_str.empty())
        {
            if (get_mode() == MOD::POST)
            {
                set_mode(MOD::ERROR, 1);
                return 0;
            }
            return 1;
        }
        int len;
        try
        {
            len = stoi(len_str);
        }
        catch (const std::exception&)
        {
            set_mode(MOD::ERROR, 1);
            return 0;
        }
        if (len < 0)
        {
            set_mode(MOD::ERROR, 1);
            return 0;
        }
        set_length(len);
        if (is_erroring())
            return 1;

        if (get_mode() != MOD::POST && len != 0)
        {
            set_mode(MOD::ERROR, 1);
            return 0;
        }
        return 1;
    }

    char Request::check_httptwo()
    {
        std::string prospect = get_header("HTTP2-Settings");
        if (!prospect.empty())
        {
            set_mode(MOD::OBSOLETE, 1);
            return 0;
        }
        return 1;
    }

    bool Request::operator()(const char* str, size_t n)
    {
        std::string prospect(str, n);
        if (headed)
        {
            body += prospect;
            if (length < body.size())
            {
                set_mode(MOD::ERROR_DOUBLE_REQUEST_FAILED, 1);
                return true;
            } // eventually add extra treatment for multiple request

            if (length > body.size())
            {
                return false;
            }
            return true;
        }
        else
        {
            head += prospect;
            auto split = prospect.find(std::string(http_crlfx2));
            if (split != std::string::npos)
            {
                size_t cur = 0;
                headed = true;

                get_mode_str(head, cur);
                get_http_version(head, cur);

                cur = head.find_first_not_of(' ', cur);
                int n_cur = head.find_first_of(' ', cur);
                if (cur != std::string::npos)
                    parse_uri(head.substr(cur, n_cur - cur));

                cur = head.find_first_not_of(' ', n_cur);
                n_cur = head.find_first_of('\r', cur);
                if (cur != std::string::npos)
                    version = head.substr(cur, n_cur - cur);
                get_headers_str(head, cur);
                if ((check_httptwo()) == 0)
                    return true;
                if ((check_length()) == 0)
                    return true;
                if (mode == MOD::GET || mode == MOD::HEAD)
                    return true;

                ///////////        get extra body                      ////////
                body = prospect.substr(split + 4, std::string::npos);
                // npos is for all char til' the end
                if (length < body.size())
                {
                    set_mode(MOD::ERROR_DOUBLE_REQUEST_FAILED, 1);
                    return true; // eventually add traitment for multiple
                                 // request
                }
                if (length > body.size())
                    return false;
                return true;
                ///////////////////////////////////////////////////////////////
            }
            return false;
        }
    }
} // namespace http
