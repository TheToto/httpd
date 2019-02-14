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
        r.set_mode("ERROR");
        cur = -1;
    }
    return cur;
}

int get_host_str(Request& r, const std::string& asked, int& cur)
{
    size_t len = asked.size();
    bool found = 0;
    while (len - cur > 0 && !found)
    {
        cur = asked.find_first_of('\n', cur) + 1;
        if (len - cur < 6 || cur == 0)
        {
            r.set_mode("ERROR");
            return -1;
        }
        if (asked.substr(cur, 6) == "Host: ")
            found = 1;
    }
    if (!found)
    {
        r.set_mode("ERROR");
        return -1;
    }
    cur += 6;
    return cur;
}

Request::Request(std::string asked)
{
    int cur = 0;
    if (get_mode_str(*this, asked, cur) < 0)
        return;
    cur = asked.find_first_not_of(' ', cur);
    int n_cur = asked.find_first_of(' ', cur);
    uri = asked.substr(cur, n_cur - cur);
    if (get_host_str(*this, asked, cur) < 0)
        return;
    cur = asked.find_first_not_of(' ', cur);
    n_cur = asked.find_first_of('\n', cur);
    host = asked.substr(cur, n_cur - cur);
}
