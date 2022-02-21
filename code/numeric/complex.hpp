#pragma once

#include <bits/stdc++.h>
using namespace std;

// A polyfill of std::complex, in case it is too slow because bananas or is not available

inline namespace complex_polyfill {

template <typename T>
struct my_complex {
    using self = my_complex<T>;
    T x, y;
    my_complex(T x = T(0), T y = T(0)) : x(x), y(y) {}

    T& real() { return x; }
    T& imag() { return y; }
    const T& real() const { return x; }
    const T& imag() const { return y; }
    friend auto real(const self& a) { return a.x; }
    friend auto imag(const self& a) { return a.y; }
    friend auto abs(const self& a) { return sqrt(norm(a)); }
    friend auto arg(const self& a) { return atan2(a.y, a.x); }
    friend auto norm(const self& a) { return a.x * a.x + a.y * a.y; }
    friend auto conj(const self& a) { return self(a.x, -a.y); }
    friend auto inv(const self& a) {
        auto n = norm(a);
        return self(a.x / n, -a.y / n);
    }

    friend auto polar(const T& r, const T& theta = T()) {
        return self(r * cos(theta), r * sin(theta));
    }

    self& operator+=(const self& b) { return *this = *this + b; }
    self& operator-=(const self& b) { return *this = *this - b; }
    self& operator*=(const self& b) { return *this = *this * b; }
    self& operator/=(const self& b) { return *this = *this / b; }

    friend self operator+(const self& a) { return a; }
    friend self operator-(const self& a) { return -a; }
    friend self operator+(const self& a, const self& b) {
        return self(a.x + b.x, a.y + b.y);
    }
    friend self operator-(const self& a, const self& b) {
        return self(a.x - b.x, a.y - b.y);
    }
    friend self operator*(const self& a, const self& b) {
        return self(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
    }
    friend self operator/(self a, self b) { return a * inv(b); }
};

} // namespace complex_polyfill
