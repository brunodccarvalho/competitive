#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T>
struct rangenum {
    static_assert(is_integral<T>::value);
    array<T, 2> n = {};

    rangenum() = default;
    rangenum(array<T, 2> ab) : n(ab) {}
    rangenum(T v) : n({v, v}) {}
    rangenum(T a, T b) : n({a, b}) {}

    bool overlaps(rangenum x) const { return n[0] <= x[1] && x[0] <= n[1]; }
    bool contains(rangenum x) const { return n[0] <= x[0] && x[1] <= n[1]; }
    bool empty() const { return n[0] > n[1]; }
    T& operator[](int x) { return n[x]; }
    const T& operator[](int x) const { return n[x]; }

    rangenum rev() const { return rangenum(n[1], n[0]); }
    rangenum& reversed() { return *this = rev(); }

    rangenum operator-() const { return rangenum(-n[0], -n[1]); }
    rangenum operator+() const { return rangenum(n[0], n[1]); }
    rangenum operator++(int) const { return rangenum(n[0]++, n[1]++); }
    rangenum operator--(int) const { return rangenum(n[0]--, n[1]--); }
    rangenum& operator++() const { return ++n[0], ++n[1], *this; }
    rangenum& operator--() const { return --n[0], --n[1], *this; }
    rangenum& operator+=(rangenum v) { return n[0] += v[0], n[1] += v[1], *this; }
    rangenum& operator-=(rangenum v) { return n[0] -= v[0], n[1] -= v[1], *this; }
    rangenum& operator*=(rangenum v) { return n[0] *= v[0], n[1] *= v[1], *this; }
    rangenum& operator/=(rangenum v) { return n[0] /= v[0], n[1] /= v[1], *this; }
    rangenum& operator&=(rangenum v) {
        return n[0] = max(n[0], v[0]), n[1] = min(n[1], v[1]), *this;
    }
    rangenum& operator|=(rangenum v) {
        return n[0] = min(n[0], v[0]), n[1] = max(n[1], v[1]), *this;
    }

    friend rangenum operator+(rangenum lhs, rangenum rhs) { return lhs += rhs; }
    friend rangenum operator-(rangenum lhs, rangenum rhs) { return lhs -= rhs; }
    friend rangenum operator*(rangenum lhs, rangenum rhs) { return lhs *= rhs; }
    friend rangenum operator/(rangenum lhs, rangenum rhs) { return lhs /= rhs; }
    friend rangenum operator&(rangenum lhs, rangenum rhs) { return lhs &= rhs; }
    friend rangenum operator|(rangenum lhs, rangenum rhs) { return lhs |= rhs; }

    friend string to_string(const rangenum& v) {
        return "(" + to_string(v[0]) + "," + to_string(v[1]) + ")";
    }
    friend bool operator==(rangenum lhs, rangenum rhs) { return lhs.n == rhs.n; }
    friend bool operator!=(rangenum lhs, rangenum rhs) { return lhs.n != rhs.n; }
    friend bool operator<(rangenum lhs, rangenum rhs) { return lhs.n < rhs.n; }
    friend bool operator>(rangenum lhs, rangenum rhs) { return lhs.n > rhs.n; }
    friend bool operator<=(rangenum lhs, rangenum rhs) { return lhs.n <= rhs.n; }
    friend bool operator>=(rangenum lhs, rangenum rhs) { return lhs.n >= rhs.n; }
    friend ostream& operator<<(ostream& out, rangenum v) { return out << to_string(v); }
    friend istream& operator>>(istream& in, rangenum& v) { return in >> v[0] >> v[1]; }
};
