#include "request/request.hh"

namespace http
{
    int Request::get_mode_str(const std::string& asked, int& cur)
    {
        size_t len = asked.size();
        if (len >= 4 && asked.substr(0, 4) == "GET ")
        {
            set_mode("GET");
            cur = 3;
        }
        else if (len >= 5 && asked.substr(0, 5) == "POST ")
        {
            set_mode("POST");
            cur = 4;
        }
        else if (len >= 5 && asked.substr(0, 5) == "HEAD ")
        {
            set_mode("HEAD");
            cur = 4;
        }
        else
        {
            set_mode("ERROR METHOD");
            set_erroring(1);
            cur = -1;
        }
        return cur;
    }

    char Request::get_http_version(const std::string& asked, int& cur)
    {
        auto pos2 = asked.find("HTTP", cur);
        if (pos2 == std::string::npos)
        {
            set_mode("ERROR");
            set_erroring(1);
            return 0;
        }

        auto p_version = asked.substr(pos2, pos2 + 8);
        if (p_version == "HTTP/1.1")
            return 1;

        if ((p_version[5] <= '0' && p_version[5] > '9') || (p_version[5] <= '0'
                    && p_version[7] > '9'))
        {
            set_mode("ERROR");
            set_erroring(1);
            return 0;
        }

        if (p_version[5] == '0' || p_version[7] == '0')
        {
            set_mode("UPGRADE");
            set_erroring(1);
            return 0;
        }

        if (p_version[5] > '1' || p_version[7] > '1')
        {
            set_mode("OBSOLETE");
            set_erroring(1);
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

    int Request::get_headers_str(const std::string& asked, int& cur)
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
            if (!is_erroring())
                set_mode("ERROR");
            return -1;
        }
        return cur;
    }

    char Request::check_length()
    {
        auto len_str = get_header("Content-Length");
        if (len_str.empty())
        {
            if (get_mode() == "POST")
            {
                set_mode("ERROR");
                set_erroring(1);
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
            set_mode("ERROR");
            set_erroring(1);
            return 0;
        }
        if (len < 0)
        {
            set_mode("ERROR");
            set_erroring(1);
            return 0;
        }
        set_length(len);
        if (is_erroring())
            return 1;

        if (get_mode() != "POST" && len != 0)

        {
            set_mode("ERROR");
            set_erroring(1);
            return 0;
        }
        return 1;
    }

    char Request::check_httptwo()
    {
        std::string prospect = get_header("HTTP2-Settings");
        if (!prospect.empty())
        {
            set_mode("OBSOLETE");
            set_erroring(1);
            return 0;
        }
        return 1;
    }

    bool Request::operator()(const char *str, size_t n)
    {
        std::string prospect(str, n);
        if (headed)
        {
            body += prospect;
            if (length < body.size())
            {
                mode = "ERROR DOUBLE REQUEST FAILED";
                erroring = 1;
                return true;
            }//eventually add extra treatment for multiple request

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
                int cur = 0;
                if (get_mode_str(head, cur) < 0)
                    return true;

                if (get_http_version(head, cur) == 0)
                    return true;

                cur = head.find_first_not_of(' ', cur);
                int n_cur = head.find_first_of(' ', cur);
                uri = head.substr(cur, n_cur - cur);
                cur = head.find_first_not_of(' ', n_cur);
                n_cur = head.find_first_of('\r', cur);
                version = head.substr(cur, n_cur - cur);
                get_headers_str(head, cur);
                if ((check_httptwo()) == 0)
                    return true;
                if ((check_length()) == 0)
                    return true;
                if (mode == "GET" || mode == "HEAD")
                    return true;

///////////        get extra body                      ////////
                body = prospect.substr(split + 4, std::string::npos);
                //npos is for all char til' the end
                if (length < body.size())
                {
                    mode = "ERROR DOUBLE REQUEST FAILED";
                        erroring = 1;
                    return true;//eventually add traitment for multiple request
                }
                if (length > body.size())
                {
                    return false;
                }
                return true;
///////////////////////////////////////////////////////////////
            }
            return false;
        }
    }
} // namespace http
