#pragma once
#include <chrono>
#include <vector>
#include <string>

namespace JACEA
{
    static inline auto get_time_ms()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    // https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    static inline std::vector<std::string> split_string(const std::string &str, const std::string &delim)
    {
        std::vector<std::string> tokens;
        size_t prev = 0, pos = 0;
        do
        {
            pos = str.find(delim, prev);
            if (pos == std::string::npos)
                pos = str.length();
            std::string token = str.substr(prev, pos - prev);
            if (!token.empty())
                tokens.push_back(token);
            prev = pos + delim.length();
        } while (pos < str.length() && prev < str.length());
        return tokens;
    }

    static inline unsigned long long bswap64(unsigned long long x)
    {
        return ((x & 0xff00000000000000ull) >> 56) |
               ((x & 0x00ff000000000000ull) >> 40) |
               ((x & 0x0000ff0000000000ull) >> 24) |
               ((x & 0x000000ff00000000ull) >> 8) |
               ((x & 0x00000000ff000000ull) << 8) |
               ((x & 0x0000000000ff0000ull) << 24) |
               ((x & 0x000000000000ff00ull) << 40) |
               ((x & 0x00000000000000ffull) << 56);
    }
}
