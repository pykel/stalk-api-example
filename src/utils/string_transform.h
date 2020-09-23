#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <boost/algorithm/string.hpp>

namespace Utils {
namespace String {

inline std::string toLower(const std::string& s)
{
    std::string ret(s.size(), ' ');
    std::transform(s.begin(), s.end(), ret.begin(), [](unsigned char c) { return std::tolower(c); });
    return ret;
}

inline std::string toUpper(const std::string& s)
{
    std::string ret(s.size(), ' ');
    std::transform(s.begin(), s.end(), ret.begin(), [](unsigned char c) { return std::toupper(c); });
    return ret;
}

inline std::string remove_if(const std::string& s, std::function<bool(const char)> predicate)
{
    std::string str = s;
    str.erase(std::remove_if(str.begin(), str.end(), predicate), str.end());
    return str;
}

inline std::string replace_all(const std::string& s, const std::vector<std::pair<std::string_view, std::string_view>>& subs)
{
    std::string ret = s;

    for (const auto& sub : subs)
    {
        boost::algorithm::replace_all(ret, sub.first, sub.second);
    }

    return ret;
}


} }
