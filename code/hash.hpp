#pragma once

#include <bits/stdc++.h>
using namespace std;

struct Hasher {
    template <typename Container>
    size_t operator()(const Container& vec) const noexcept {
        using std::hash;
        hash<typename Container::value_type> hasher;
        size_t seed = distance(begin(vec), end(vec));
        for (const auto& n : vec) {
            seed ^= hasher(n) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
    template <typename U, typename V>
    size_t operator()(const pair<U, V>& p) const noexcept {
        using std::hash;
        size_t a = hash<U>{}(p.first), b = hash<V>{}(p.second);
        return (a + b) * (a + b + 1) / 2 + b;
    }
    template <typename U>
    size_t operator()(const array<U, 2>& p) const noexcept {
        using std::hash;
        hash<U> hasher;
        size_t a = hasher(p[0]), b = hasher(p[1]);
        return (a + b) * (a + b + 1) / 2 + b;
    }
    template <int i, typename Tuple>
    size_t tuple_compute(const Tuple& tuple) const noexcept {
        if constexpr (i == std::tuple_size_v<Tuple>) {
            return std::tuple_size_v<Tuple>;
        } else {
            using std::hash;
            hash<std::tuple_element_t<i, Tuple>> hasher;
            size_t seed = tuple_compute<i + 1, Tuple>(tuple);
            size_t h = hasher(std::get<i>(tuple));
            return seed ^ (h + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        }
    }
    template <typename... Ts>
    size_t operator()(const tuple<Ts...>& t) const noexcept {
        return tuple_compute<0, tuple<Ts...>>(t);
    }
};

namespace std {

template <typename T, size_t N>
struct hash<array<T, N>> : Hasher {};
template <typename T>
struct hash<vector<T>> : Hasher {};
template <typename U, typename V>
struct hash<pair<U, V>> : Hasher {};
template <typename... Ts>
struct hash<tuple<Ts...>> : Hasher {};

} // namespace std

struct Int128Hasher {
    size_t operator()(__uint128_t x) const noexcept {
        array<uint64_t, 2>* arr = reinterpret_cast<array<uint64_t, 2>*>(&x);
        return Hasher{}(*arr);
    }
    size_t operator()(__int128_t x) const noexcept {
        array<uint64_t, 2>* arr = reinterpret_cast<array<uint64_t, 2>*>(&x);
        return Hasher{}(*arr);
    }
};

namespace std {

template <>
struct hash<__int128_t> : Hasher {};
template <>
struct hash<__uint128_t> : Hasher {};

} // namespace std

struct rolling_hasher {
    static constexpr size_t base = 2001539UL;
    static constexpr size_t mask = (1 << 26) - 1;
    size_t n, mul;

    explicit rolling_hasher(size_t n) : n(n), mul(powovf(n) & mask) {}

    size_t operator()(const char* s, const char* e) const noexcept {
        size_t seed = 0;
        while (s != e) {
            seed = (seed * base + *s++) & mask;
        }
        return seed;
    }

    size_t operator()(const string& s) const noexcept {
        return (*this)(s.data(), s.data() + s.length());
    }

    size_t roll(size_t seed, unsigned char out, unsigned char in) const noexcept {
        return (seed * base + in + (mask + 1 - out) * mul) & mask;
    }

    static constexpr size_t powovf(size_t e) {
        size_t power = 1, b = base;
        while (e) {
            if (e & 1) {
                power = power * b;
            }
            e >>= 1;
            b = b * b;
        }
        return power;
    }
};
