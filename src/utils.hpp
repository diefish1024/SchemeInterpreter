#pragma once

namespace util {
    template<typename T>
    inline T gcd(T a, T b) {
        if (a < 0) a = -a;
        if (b < 0) b = -b;
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }

    template<typename T>
    inline T lcm(T a, T b) {
        return std::abs(a * b) / gcd(a, b);
    }

    template<typename T>
    inline T normalize_rational(T& num, T& den) {
        int g = gcd(num, den);
        num /= g;
        den /= g;
        return g;
    }
}