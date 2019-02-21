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
        size_t pos;
        try
        {
            pos = asked.find("/", cur);
        }
        catch(const std::exception&)
        {
            r.set_mode("ERROR");
            r.set_erroring(1);
            return -1;
        }
        if (pos == std::string::npos)
        {
            r.set_mode("ERROR");
            r.set_erroring(1);
            return -1;
        }

        size_t pos2;
        try
        {
            pos2 = asked.find("HTTP", cur);
        }
        catch(const std::exception&)
        {
            r.set_mode("ERROR");
            r.set_erroring(1);
            return -1;
        }
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
        std::string version;
        try
        {
            version = asked.substr(pos2, pos2 + 8);
        }
        catch (const std::exception&)
        {
                r.set_mode("ERROR");
                r.set_erroring(1);
                return -1;
        }
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
            if (len_v > 0)
                r.set_header(asked.substr(cur, len_cur),
                             asked.substr(beg_v, len_v));
            else
                r.set_header(asked.substr(cur, len_cur), asked.substr(beg_v));
        }
        if (!found)
        {
            if (!r.is_erroring())
                r.set_mode("ERROR");
            return -1;
        }
        return cur;
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
    }

} // namespace http
