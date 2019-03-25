#include "base64.hh"
#include <iostream>

static std::string translate(std::string& res, char tmp)
{
    if (tmp < 26)
        res.push_back('A' + tmp);
    else if (tmp < 52)
        res.push_back('a' + tmp - 26);
    else if (tmp < 62)
        res.push_back('0' + tmp - 52);
    else if (tmp == 62)
        res.push_back('+');
    else
        res.push_back('/');
    return res;
}

std::string to_64(const std::string& target)
{
    const char *str = target.c_str();
    int size = target.size();
    std::string res = "";

    int cur = 0;

    char tab1[3];
    char tab2[4];

    for (; cur < size - 2; cur += 3)
    {
        tab1[0] = str[cur];
        tab1[1] = str[cur + 1];
        tab1[2] = str[cur + 2];
        tab2[0] = (tab1[0] & 0xFC) >> 2;
        tab2[1] = ((tab1[0] & 0x03) << 4) | ((tab1[1] & 0xF0) >> 4);
        tab2[2] = ((tab1[1] & 0x0F) << 2) | ((tab1[2] & 0xC0) >> 6);
        tab2[3] = tab1[2] & 0x3F;

        translate(res, tab2[0]);
        translate(res, tab2[1]);
        translate(res, tab2[2]);
        translate(res, tab2[3]);
    }

    if (cur == size - 2)
    {
        tab1[0] = str[cur];
        tab1[1] = str[cur + 1];
        tab2[0] = (tab1[0] & 0xFC) >> 2;
        tab2[1] = ((tab1[0] & 0x03) << 4) | ((tab1[1] & 0xF0) >> 4);
        tab2[2] = (tab1[1] & 0x0F) << 2;


        translate(res, tab2[0]);
        translate(res, tab2[1]);
        translate(res, tab2[2]);

        res.push_back('=');
    }
    else if (cur == size - 1)
    {
        tab1[0] = str[cur];
        tab2[0] = (tab1[0] & 0xFC) >> 2;
        tab2[1] = (tab1[0] & 0x03) << 4;
        res = translate(res, tab2[0]);
        res = translate(res, tab2[1]);

        res.push_back('=');
        res.push_back('=');
    }

    return res;
}
