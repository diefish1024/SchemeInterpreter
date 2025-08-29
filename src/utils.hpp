#pragma once
#include <algorithm>

namespace util {
    inline int gcd(int a, int b) {
        if (a < 0) a = -a;
        if (b < 0) b = -b;
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    inline int lcm(int a, int b) {
        return std::abs(a * b) / gcd(a, b);
    }

    inline int normalize_rational(int& num, int& den) {
        int g = gcd(num, den);
        num /= g;
        den /= g;
        return g;
    }
}