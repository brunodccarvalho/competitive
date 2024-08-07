#pragma once

#include "hash.hpp"

using int128_t = __int128_t;

int128_t gcd(int128_t a, int128_t b) { return b == 0 ? a : gcd(b, a % b); }
int128_t lcm(int128_t a, int128_t b) { return (a / gcd(a, b)) * b; }

int128_t to_int128(const string& s) {
    int128_t val = 0;
    int S = s.size();
    for (int i = 0; i < S; i++) {
        if ('0' <= s[i] && s[i] <= '9') {
            val = 10 * val + (s[i] - '0');
        } else {
            break;
        }
    }
    return val;
}

namespace std {
string to_string(int128_t x) {
    if (x == 0)
        return "0";
    __uint128_t k = x;
    if (k == (__uint128_t(1) << 127))
        return "-170141183460469231731687303715884105728";
    string result;
    if (x < 0) {
        result += "-";
        x *= -1;
    }
    string t;
    while (x) {
        t.push_back('0' + x % 10);
        x /= 10;
    }
    reverse(t.begin(), t.end());
    return result + t;
}

ostream& operator<<(ostream& o, int128_t x) { return o << to_string(x); }
istream& operator>>(istream& o, int128_t& x) {
    string s;
    return o >> s, x = to_int128(s), o;
}
} // namespace std

struct Int128Hasher {
    size_t operator()(__uint128_t x) const noexcept {
        array<uint64_t, 2>* arr = reinterpret_cast<array<uint64_t, 2>*>(&x);
        return Hasher{}(*arr);
    }
    size_t operator()(int128_t x) const noexcept {
        array<uint64_t, 2>* arr = reinterpret_cast<array<uint64_t, 2>*>(&x);
        return Hasher{}(*arr);
    }
};

namespace std {

template <>
struct hash<int128_t> : Int128Hasher {};
template <>
struct hash<__uint128_t> : Int128Hasher {};

} // namespace std
