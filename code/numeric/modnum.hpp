#pragma once

#include <bits/stdc++.h>
using namespace std;

template <uint32_t mod>
struct modnum {
    // change these if you need another size of integers
    static constexpr inline uint32_t MOD = mod;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i32 = int32_t;
    using i64 = int64_t;
    static_assert(MOD > 0 && MOD < UINT_MAX / 2);

    u32 n;

    constexpr modnum() : n(0) {}
    constexpr modnum(u64 v) : n(v >= MOD ? v % MOD : v) {}
    constexpr modnum(u32 v) : n(v >= MOD ? v % MOD : v) {}
    constexpr modnum(i64 v) : modnum(v >= 0 ? u64(v) : u64(MOD + v % int(MOD))) {}
    constexpr modnum(i32 v) : modnum(v >= 0 ? u32(v) : u32(MOD + v % int(MOD))) {}
    explicit constexpr operator i32() const { return n; }
    explicit constexpr operator u32() const { return n; }
    explicit constexpr operator bool() const { return n != 0; }

    static constexpr u32 fit(u32 x) { return x >= MOD ? x - MOD : x; }
    static constexpr int modinv(u32 x) {
        int nx = 1, ny = 0;
        u32 y = MOD;
        while (x) {
            auto k = y / x;
            y = y % x;
            ny = ny - k * nx;
            swap(x, y), swap(nx, ny);
        }
        return ny < 0 ? MOD + ny : ny;
    }
    friend constexpr modnum modpow(modnum b, int64_t e) {
        modnum p = 1;
        while (e > 0) {
            if (e & 1)
                p = p * b;
            if (e >>= 1)
                b = b * b;
        }
        return p;
    }

    constexpr modnum inv() const { return modinv(n); }
    constexpr modnum operator-() const { return n == 0 ? n : MOD - n; }
    constexpr modnum operator+() const { return *this; }
    constexpr modnum operator++(int) { return n = fit(n + 1), *this - 1; }
    constexpr modnum operator--(int) { return n = fit(MOD + n - 1), *this + 1; }
    constexpr modnum &operator++() { return n = fit(n + 1), *this; }
    constexpr modnum &operator--() { return n = fit(MOD + n - 1), *this; }
    constexpr modnum &operator+=(modnum v) { return n = fit(n + v.n), *this; }
    constexpr modnum &operator-=(modnum v) { return n = fit(MOD + n - v.n), *this; }
    constexpr modnum &operator*=(modnum v) { return n = (u64(n) * v.n) % MOD, *this; }
    constexpr modnum &operator/=(modnum v) { return *this *= v.inv(); }

    friend constexpr modnum operator+(modnum lhs, modnum rhs) { return lhs += rhs; }
    friend constexpr modnum operator-(modnum lhs, modnum rhs) { return lhs -= rhs; }
    friend constexpr modnum operator*(modnum lhs, modnum rhs) { return lhs *= rhs; }
    friend constexpr modnum operator/(modnum lhs, modnum rhs) { return lhs /= rhs; }

    friend string to_string(modnum v) { return to_string(v.n); }
    friend constexpr bool operator==(modnum lhs, modnum rhs) { return lhs.n == rhs.n; }
    friend constexpr bool operator!=(modnum lhs, modnum rhs) { return lhs.n != rhs.n; }
    friend ostream &operator<<(ostream &out, modnum v) { return out << v.n; }
    friend istream &operator>>(istream &in, modnum &v) {
        i64 n;
        return in >> n, v = modnum(n), in;
    }
};

namespace std {

template <uint32_t MOD>
struct hash<modnum<MOD>> {
    size_t operator()(modnum<MOD> x) const noexcept { return std::hash<int>{}(x); }
};

} // namespace std

struct dmodnum {
    // change these if you need another size of integers
    static inline uint32_t MOD = 0;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i32 = int32_t;
    using i64 = int64_t;

    u32 n;

    dmodnum() : n(0) {}
    dmodnum(u64 v) : n(v >= MOD ? v % MOD : v) {}
    dmodnum(u32 v) : n(v >= MOD ? v % MOD : v) {}
    dmodnum(i64 v) : dmodnum(v >= 0 ? u64(v) : u64(MOD + v % int(MOD))) {}
    dmodnum(i32 v) : dmodnum(v >= 0 ? u32(v) : u32(MOD + v % int(MOD))) {}
    explicit operator i32() const { return n; }
    explicit operator u32() const { return n; }
    explicit operator bool() const { return n != 0; }

    static u32 fit(u32 x) { return x >= MOD ? x - MOD : x; }
    static int modinv(u32 x) {
        int nx = 1, ny = 0;
        u32 y = MOD;
        while (x) {
            auto k = y / x;
            y = y % x;
            ny = ny - k * nx;
            swap(x, y), swap(nx, ny);
        }
        return ny < 0 ? MOD + ny : ny;
    }
    friend dmodnum modpow(dmodnum b, int64_t e) {
        dmodnum p = 1;
        while (e > 0) {
            if (e & 1)
                p = p * b;
            if (e >>= 1)
                b = b * b;
        }
        return p;
    }

    dmodnum inv() const { return modinv(n); }
    dmodnum operator-() const { return n == 0 ? n : MOD - n; }
    dmodnum operator+() const { return *this; }
    dmodnum operator++(int) { return n = fit(n + 1), *this - 1; }
    dmodnum operator--(int) { return n = fit(MOD + n - 1), *this + 1; }
    dmodnum &operator++() { return n = fit(n + 1), *this; }
    dmodnum &operator--() { return n = fit(MOD + n - 1), *this; }
    dmodnum &operator+=(dmodnum v) { return n = fit(n + v.n), *this; }
    dmodnum &operator-=(dmodnum v) { return n = fit(MOD + n - v.n), *this; }
    dmodnum &operator*=(dmodnum v) { return n = (u64(n) * v.n) % MOD, *this; }
    dmodnum &operator/=(dmodnum v) { return *this *= v.inv(); }

    friend dmodnum operator+(dmodnum lhs, dmodnum rhs) { return lhs += rhs; }
    friend dmodnum operator-(dmodnum lhs, dmodnum rhs) { return lhs -= rhs; }
    friend dmodnum operator*(dmodnum lhs, dmodnum rhs) { return lhs *= rhs; }
    friend dmodnum operator/(dmodnum lhs, dmodnum rhs) { return lhs /= rhs; }

    friend string to_string(dmodnum v) { return to_string(v.n); }
    friend bool operator==(dmodnum lhs, dmodnum rhs) { return lhs.n == rhs.n; }
    friend bool operator!=(dmodnum lhs, dmodnum rhs) { return lhs.n != rhs.n; }
    friend ostream &operator<<(ostream &out, dmodnum v) { return out << v.n; }
    friend istream &operator>>(istream &in, dmodnum &v) {
        i64 n;
        return in >> n, v = dmodnum(n), in;
    }
};
