#pragma once

#include "numeric/modnum.hpp"

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

struct polyhasher {
    using V1 = modnum<998244353>;
    using V2 = modnum<999999893>;
    static inline vector<int> B1list = {3, 5, 6, 10, 11, 12, 13, 20, 22, 24, 26, 27, 35};
    static inline vector<int> B2list = {2, 3, 5, 8, 11, 12, 14, 18, 20, 21, 23, 26, 27};
    static inline int off = 73;
    static inline V1 B1 = 5, R1;
    static inline V2 B2 = 5, R2;
    using Hash = tuple<V1, V2, int>;

    static V1 get1(int c) { return V1(c + off); }
    static V2 get2(int c) { return V2(c + off); }

    static inline vector<Hash> cache;
    static void init(int N) {
        uniform_int_distribution<int> b1dist(0, B1list.size() - 1);
        uniform_int_distribution<int> b2dist(0, B2list.size() - 1);
        uint64_t RANDOM = chrono::steady_clock::now().time_since_epoch().count();
        mt19937 rng(RANDOM);
        B1 = B1list[b1dist(rng)];
        B2 = B2list[b2dist(rng)];
        R1 = 1 / B1, R2 = 1 / B2;
        cache.resize(N + 1);
        V1 h1 = 1;
        V2 h2 = 1;
        for (int i = 0; i <= N; i++) {
            cache[i] = {h1, h2, i};
            h1 *= B1, h2 *= B2;
        }
    }

    template <typename T>
    static Hash extend_right(Hash a, T c) { // a c -> [ac]
        auto [a1, a2, as] = a;
        auto [h1, h2, _] = cache[as];
        Hash h = {a1 + get1(c) * h1, a2 + get2(c) * h2, as + 1};
        return h;
    }

    template <typename T>
    static Hash extend_left(Hash a, T c) { // c a -> [ca]
        auto [a1, a2, as] = a;
        Hash h = {a1 * B1 + get1(c), a2 * B2 + get2(c), as + 1};
        return h;
    }

    template <typename T>
    static Hash trim_right(Hash a, T c) { // [ac] -> a
        auto [a1, a2, as] = a;
        auto [h1, h2, _] = cache[as];
        assert(as >= 1);
        Hash h = {a1 - get1(c) * h1, a2 - get2(c) * h2, as - 1};
        return h;
    }

    template <typename T>
    static Hash trim_left(Hash a, T c) { // [ca] -> a
        auto [a1, a2, as] = a;
        assert(as >= 1);
        Hash h = {(a1 - get1(c)) * R1, (a2 - get2(c)) * R2, as - 1};
        return h;
    }

    static Hash merge(Hash a, Hash b) { // a b -> [ab]
        auto [a1, a2, as] = a;
        auto [b1, b2, bs] = b;
        auto [h1, h2, _] = cache[as];
        Hash h = {a1 + b1 * h1, a2 + b2 * h2, as + bs};
        return h;
    }

    static Hash trim_right(Hash a, Hash b) { // [ab] -> a
        auto [a1, a2, as] = a;
        auto [b1, b2, bs] = b;
        auto [h1, h2, _] = cache[as];
        assert(as >= bs);
        Hash h = {a1 - b1 * h1, a2 - b2 * h2, as - bs};
        return h;
    }

    static Hash trim_left(Hash a, Hash b) { // [ba] -> a
        auto [a1, a2, as] = a;
        auto [b1, b2, bs] = b;
        auto [h1, h2, _] = cache[bs];
        assert(as >= bs);
        Hash h = {(a1 - b1) / h1, (a2 - b2) / h2, as - bs};
        return h;
    }

    template <typename T>
    static Hash make(T c) {
        Hash h = {get1(c), get2(c), 1};
        return h;
    }

    static Hash make(const string& s) {
        Hash h = {0, 0, 0};
        for (int i = 0, S = s.size(); i < S; i++) {
            h = extend_right(h, s[i]);
        }
        return h;
    }

    template <typename T>
    static Hash make(const vector<T>& s) {
        Hash h = {0, 0, 0};
        for (int i = 0, S = s.size(); i < S; i++) {
            h = extend_right(h, s[i]);
        }
        return h;
    }
};
