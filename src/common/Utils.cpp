#include "common/Utils.h"
#include <cctype>

std::string Utils::ToLower(const std::string& s)
{
    std::string res = s;
    for (char& ch : res) ch = static_cast<char>(std::tolower(ch));
    return res;
}

bool Utils::IsNumber(const std::string& s)
{
    if (s.empty()) return false;
    for (char c : s)
        if (!std::isdigit(c) && c != '.') return false;
    return true;
}
