#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T>
struct quadratic {
    T a, b, c; // f(x) = ax^2 + bx + c

    quadratic() : a(0), b(0), c(0) {}
    quadratic(T a, T b, T c) : a(move(a)), b(move(b)), c(move(c)) {}

    T eval(const T& x) const { return (a * x + b) * x + c; }
    T operator()(const T& x) const { return eval(x); }

    T argvertex() const { return -b / (2 * a); }
    T deriv(T x) const { return 2 * a * x + b; }

    // Minimum of the parabola in the range [L,R]
    T argmin(const T& l, const T& r) const {
        if (a > 0) {
            if (auto x = argvertex(); l <= x && x <= r) {
                return x;
            }
        }
        return eval(l) <= eval(r) ? l : r;
    }

    // Maximum of the parabola in the range [L,R]
    T argmax(const T& l, const T& r) const {
        if (a < 0) {
            if (auto x = argvertex(); l <= x && x <= r) {
                return x;
            }
        }
        return eval(l) >= eval(r) ? l : r;
    }

    T min(const T& l, const T& r) const { return eval(argmin(l, r)); }
    T max(const T& l, const T& r) const { return eval(argmax(l, r)); }
    T min() const { return assert(a > 0), eval(argvertex()); }
    T max() const { return assert(a < 0), eval(argvertex()); }

    // Shift parabola to the right by k, new f(x) := f(x-k)
    auto& shift(T k) {
        c += k * (a * k - b);
        b -= 2 * a * k;
        return *this;
    }

    // Dilate parabola by k, new f(x) := f(kx)
    auto& dilate(T k) {
        a *= k * k;
        b *= k;
        return *this;
    }

    auto& flip() {
        b = -b;
        return *this;
    }

    friend quadratic operator-(const quadratic& fn) {
        return quadratic(-fn.a, -fn.b, -fn.c);
    }
    friend quadratic operator+(const quadratic& fn) {
        return quadratic(fn.a, fn.b, fn.c);
    }
    friend quadratic operator*(const T& k, const quadratic& fn) {
        return quadratic(fn.a * k, fn.b * k, fn.c * k);
    }
    friend quadratic operator*(const quadratic& fn, const T& k) {
        return quadratic(fn.a * k, fn.b * k, fn.c * k);
    }
    friend quadratic operator/(const quadratic& fn, const T& k) {
        return quadratic(fn.a / k, fn.b / k, fn.c / k);
    }
    friend quadratic operator+(const quadratic& fn, const T& k) {
        return quadratic(fn.a, fn.b, fn.c + k);
    }
    friend quadratic operator-(const quadratic& fn, const T& k) {
        return quadratic(fn.a, fn.b, fn.c - k);
    }
    friend quadratic operator+(const quadratic& fn, const quadratic& g) {
        return quadratic(fn.a + g.a, fn.b + g.b, fn.c + g.c);
    }
    friend quadratic operator-(const quadratic& fn, const quadratic& g) {
        return quadratic(fn.a - g.a, fn.b - g.b, fn.c - g.c);
    }
    auto& operator*=(const T& k) { return a *= k, b *= k, c *= k, *this; }
    auto& operator/=(const T& k) { return a /= k, b /= k, c /= k, *this; }
    auto& operator+=(const T& k) { return c += k, *this; }
    auto& operator-=(const T& k) { return c -= k, *this; }
    auto& operator+=(const quadratic& g) { return a += g.a, b += g.b, c += g.c, *this; }
    auto& operator-=(const quadratic& g) { return a -= g.a, b -= g.b, c -= g.c, *this; }

    // Get all real roots
    template <typename D = double>
    vector<D> roots(D eps = 1e-10) const {
        D A = D(a), B = D(b), C = D(c);
        if (abs(A) <= eps) {     // assume A=0
            if (abs(B) <= eps) { // assume B=0
                return {};
            } else {
                return {-C / B};
            }
        }
        D disc = B * B - 4 * A * C;
        if (disc < 0) {
            return {};
        } else if (disc > 0) {
            using std::sqrt;
            D x0, x1;
            if (B <= 0) {
                x0 = (sqrt(disc) - B) / (2 * A);
                x1 = abs(A * x0) <= eps ? (B / A + x0) : C / (A * x0);
            } else {
                x0 = -(sqrt(disc) + B) / (2 * A);
                x1 = abs(A * x0) <= eps ? (B / A + x0) : C / (A * x0);
            }
            if (x0 <= x1) {
                return {x0, x1};
            } else {
                return {x1, x0};
            }
        } else {
            return {-B / (2 * A)};
        }
    }
};
