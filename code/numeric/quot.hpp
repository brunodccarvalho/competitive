#pragma once

#include <bits/stdc++.h>
using namespace std;

// Rational arithmetic not taking GCDs and avoiding overflow, T should be an integer type
// Positive infinity is quot(>0, 0) and negative infinity is quot(<0, 0).
template <typename T>
struct quot {
    using unit_type = T;
    T n, d; // n/d

    quot() : n(0), d(1) {}
    quot(T num) : n(num), d(1) {}
    quot(T num, T den) : n(den >= 0 ? num : -num), d(den >= 0 ? den : -den) {}

    explicit operator bool() const noexcept { return n != 0 && d != 0; }
    explicit operator T() const noexcept { return assert(d), n / d; }
    explicit operator double() const noexcept { return assert(d), double(n) / d; }

    static int sign(T x) { return x > 0 ? +1 : x < 0 ? -1 : 0; }
    static bool undefined(quot f) { return f.n == 0 && f.d == 0; }
    static int infsign(quot f) { return f.d == 0 ? sign(f.n) : 0; }

    friend quot abs(quot f) { return quot(abs(f.n), f.d); }
    friend quot inv(quot f) { return quot(f.d, f.n); }
    friend T floor(quot f) { return f.n >= 0 ? f.n / f.d : (f.n - f.d + 1) / f.d; }
    friend T ceil(quot f) { return f.n >= 0 ? (f.n + f.d - 1) / f.d : f.n / f.d; }

    friend bool operator==(quot a, quot b) { return compare(a, b) == 0; }
    friend bool operator!=(quot a, quot b) { return compare(a, b) != 0; }
    friend bool operator<(quot a, quot b) { return compare(a, b) < 0; }
    friend bool operator>(quot a, quot b) { return compare(a, b) > 0; }
    friend bool operator<=(quot a, quot b) { return compare(a, b) <= 0; }
    friend bool operator>=(quot a, quot b) { return compare(a, b) >= 0; }

    static int compare(quot a, quot b) {
        if (a.d == 0 || b.d == 0) {
            return infsign(a) - infsign(b);
        }
        T x = floor(a), y = floor(b);
        while (x == y) {
            a.n -= x * a.d;
            b.n -= y * b.d;
            if (a.n == 0 || b.n == 0) {
                return sign(a.n - b.n);
            }
            swap(a.n, b.d), swap(a.d, b.n);
            x = a.n / a.d, y = b.n / b.d;
        }
        return sign(x - y);
    }

    friend quot operator+(quot a, quot b) {
        if (a.d == b.d) {
            return quot(a.n + b.n, a.d);
        } else if (a.d == 0 || b.d == 0) {
            return a.d == 0 ? a : b;
        } else if (a.d < b.d && b.d % a.d == 0) {
            return quot(a.n * (b.d / a.d) + b.n, b.d);
        } else if (a.d > b.d && a.d % b.d == 0) {
            return quot(a.n + b.n * (a.d / b.d), a.d);
        } else {
            return quot(a.n * b.d + b.n * a.d, a.d * b.d);
        }
    }
    friend quot operator-(quot a, quot b) { return a + (-b); }
    friend quot operator*(quot a, quot b) {
        if (a.n == b.d) {
            return quot(b.n, a.d);
        } else if (a.d == b.n) {
            return quot(a.n, b.d);
        } else {
            return quot(a.n * b.n, a.d * b.d);
        }
    }
    friend quot operator/(quot a, quot b) { return a * inv(b); }
    friend quot operator%(quot a, quot b) { return a - T(a / b) * b; }
    friend quot& operator+=(quot& a, quot b) { return a = a + b; }
    friend quot& operator-=(quot& a, quot b) { return a = a - b; }
    friend quot& operator*=(quot& a, quot b) { return a = a * b; }
    friend quot& operator/=(quot& a, quot b) { return a = a / b; }
    friend quot& operator%=(quot& a, quot b) { return a = a % b; }

    friend quot operator-(quot f) { return quot(-f.n, f.d); }
    friend bool operator!(quot f) { return f.n == 0; }

    friend string to_string(quot f) {
        if (f.d == 0) {
            return f.n ? f.n > 0 ? "inf" : "-inf" : "undef";
        } else if (f.d == 1) {
            return to_string(f.n);
        } else {
            return to_string(f.n) + '/' + to_string(f.d);
        }
    }

    friend ostream& operator<<(ostream& out, quot f) { return out << to_string(f); }
};

template <typename Quot>
Quot stoquot(const string& s) {
    int i = 0, S = s.size();
    while (i < S && isspace(s[i]))
        i++; // skip whitespace
    int j = i;
    bool neg = j < S && s[j] == '-', pos = j < S && s[j] == '+';
    j += neg || pos; // skip leading sign
    typename Quot::unit_type integer = 0;
    while (j < S && '0' <= s[j] && s[j] <= '9') {
        integer = 10 * integer + (s[j] - '0'), j++; // read digits
    }
    integer = neg ? -integer : integer;
    if (i < j && j < S && s[j] == '/') {
        int n = 0, k = j + 1;
        typename Quot::unit_type denom = 0;
        while (k < S && '0' <= s[k] && s[k] <= '9') {
            denom = 10 * denom + (s[k] - '0'), n++, k++; // read digits
        }
        return n ? Quot(integer, denom) : Quot(integer);
    } else if (j < S && s[j] == '.') {
        int n = 0, k = j + 1;
        typename Quot::unit_type decimal = 0, ten = 1;
        while (k < S && '0' <= s[k] && s[k] <= '9') {
            decimal = 10 * decimal + (s[k] - '0'), ten *= 10, n++, k++; // read digits
        }
        decimal = neg ? -decimal : decimal;
        return Quot(integer * ten + decimal, ten);
    } else {
        return integer;
    }
}
template <typename T>
istream& operator>>(istream& in, quot<T>& f) {
    string s;
    return in >> s, f = stoquot<quot<T>>(s), in;
}

namespace std {

template <typename T>
struct hash<quot<T>> {
    size_t operator()(const quot<T>& f) const noexcept {
        size_t a = hash<T>{}(f.n), b = hash<T>{}(f.d);
        return (a + b) * (a + b + 1) / 2 + b;
    }
};
template <typename T>
struct numeric_limits<quot<T>> {
    static constexpr inline bool is_specialized = true, is_exact = true;
    static constexpr inline bool is_bounded = true, is_signed = true;
    static constexpr inline bool is_integer = false, has_infinity = true;
    static inline auto lowest() { return quot<T>(numeric_limits<T>::min(), 1); }
    static inline auto min() { return quot<T>(numeric_limits<T>::min(), 1); }
    static inline auto max() { return quot<T>(numeric_limits<T>::max(), 1); }
    static inline auto infinity() { return quot<T>(1, 0); }
    static inline auto epsilon() { return quot<T>(0, 1); }
};

} // namespace std
