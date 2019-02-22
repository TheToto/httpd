#include "request/request.hh"

namespace http
{
    int get_mode_str(Request& r, const std::string& asked, int& cur)
    {
        size_t len = asked.size();
        if (len >= 4 && asked.substr(0, 4) == "GET ")
        {
            r.set_mode("GET");
            cur = 3;
        }
        else if (len >= 5 && asked.substr(0, 5) == "POST ")
        {
            r.set_mode("POST");
            cur = 4;
        }
        else if (len >= 5 && asked.substr(0, 5) == "HEAD ")
        {
            r.set_mode("HEAD");
            cur = 4;
        }
        else
        {
            r.set_mode("ERROR METHOD");
            r.set_erroring(1);
            cur = -1;
        }
        return cur;
    }

    static int get_http_version(Request& r, const std::string& asked, int& cur)
    {

        auto pos = asked.find("/", cur);
        if (pos == std::string::npos)
        {
            r.set_mode("ERROR");
            r.set_erroring(1);
            return -1;
        }

        auto pos2 = asked.find("HTTP", cur);
        if (pos2 == std::string::npos || pos++ >= pos2)
        {
            r.set_mode("ERROR");
            r.set_erroring(1);
            return -1;
        }

        while (pos < pos2)
        {
            if (asked[pos++] != ' ')
            {
                r.set_mode("ERROR");
                r.set_erroring(1);
                return -1;
            }
        }

        auto version = asked.substr(pos2, pos2 + 8);
        if (version == "HTTP/1.1")
            return 1;

        if ((version[5] <= '0' && version[5] > '9') || (version[5] <= '0'
                    && version[7] > '9'))
        {
            r.set_mode("ERROR");
            r.set_erroring(1);
            return -1;
        }

        if (version[5] == '0' || version[7] == '0')
        {
            r.set_mode("UPGRADE");
            r.set_erroring(1);
            return -1;
        }

        if (version[5] > '1' || version[7] > '1')
        {
            r.set_mode("OBSOLETE");
            r.set_erroring(1);
            return -1;
        }

        return 1;
    }

    void format_header_val(std::string& h_val)
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

    int get_headers_str(Request& r, const std::string& asked, int& cur)
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
            r.set_header(name, val);
        }
        if (!found)
        {
            if (!r.is_erroring())
                r.set_mode("ERROR");
            return -1;
        }
        return cur;
    }

    static int check_length(Request& r)
    {
        auto len_str = r.get_header("Content-Length");
        if (len_str.empty())
        {
            if (r.get_mode() == "POST")
            {
                r.set_mode("ERROR");
                r.set_erroring(1);
                return -1;
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
            r.set_mode("ERROR");
            r.set_erroring(1);
            return -1;
        }
        if (len < 0)
        {
            r.set_mode("ERROR");
            r.set_erroring(1);
            return -1;
        }
        r.set_length(len);
        if (r.is_erroring())
            return 1;

        if (r.get_mode() != "POST" && len != 0)
        {
            r.set_mode("ERROR");
            r.set_erroring(1);
            return -1;
        }
        return 1;
    }

    Request::Request(std::string asked)
    {
        int cur = 0;
        if (get_mode_str(*this, asked, cur) < 0)
            return;

        if (get_http_version(*this, asked, cur) < 0)
            return;

        cur = asked.find_first_not_of(' ', cur);
        int n_cur = asked.find_first_of(' ', cur);
        uri = asked.substr(cur, n_cur - cur);
        cur = asked.find_first_not_of(' ', n_cur);
        n_cur = asked.find_first_of('\r', cur);
        version = asked.substr(cur, n_cur - cur);
        get_headers_str(*this, asked, cur);
        if ((check_length(*this)) < 0)
            return;
    }

} // namespace http
