#pragma once
#include <string>
#include <stdexcept>
#include <regex>

namespace util {
    template<typename T>
    inline T gcd(T a, T b) {
        if (a < 0) a = -a;
        if (b < 0) b = -b;
        while (b != 0) {
            T temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    template<typename T>
    inline T lcm(T a, T b) {
        return std::abs(a) / gcd(a, b) * std::abs(b);
    }

    template<typename T>
    inline T normalize_rational(T& num, T& den) {
        T g = gcd(num, den);
        num /= g;
        den /= g;
        return g;
    }

    inline bool is_valid_variable_name(const std::string& name) {
        if (name.empty()) return false;
        
        char first = name[0];
        if (std::isdigit(first) || first == '.' || first == '@') {
            return false;
        }

        for (char c : name) {
            if (c == '#' || c == '\'' || c == '\"' || c == '`') {
                return false;
            }
        }
        
        return true;
    }
}