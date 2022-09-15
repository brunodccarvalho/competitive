#pragma once

#include <bits/stdc++.h>
using namespace std;

// Rational arithmetic taking GCDs and avoiding overflow, T should be an integer type.
// Positive infinity is frac(1, 0) and negative infinity is frac(-1, 0).
template <typename T, bool SAFE = true>
struct frac {
    using unit_type = T;
    T n, d; // n/d

    frac(T a = 0, T b = 1) : n(b >= 0 ? a : -a), d(b >= 0 ? b : -b) {
        auto g = gcd(n, d);
        n /= g, d /= g;
    }

    explicit operator bool() const noexcept { return n != 0 && d != 0; }
    explicit operator T() const noexcept { return assert(d), n / d; }
    explicit operator double() const noexcept { return assert(d), double(n) / double(d); }

    static int sign(T x) { return x > 0 ? +1 : x < 0 ? -1 : 0; }
    static bool undefined(frac f) { return f.n == 0 && f.d == 0; }
    friend int infsign(frac f) { return f.d == 0 ? sign(f.n) : 0; }

    friend frac abs(frac f) { return frac(abs(f.n), f.d); }
    friend frac inv(frac f) { return frac(f.d, f.n); }
    friend T floor(frac f) { return f.n >= 0 ? f.n / f.d : (f.n - f.d + 1) / f.d; }
    friend T ceil(frac f) { return f.n >= 0 ? (f.n + f.d - 1) / f.d : f.n / f.d; }
    friend frac gcd(frac a, frac b) {
        return frac(gcd(a.n, b.n), a.d * (b.d / gcd(a.d, b.d)));
    }

    friend bool operator==(frac a, frac b) { return compare(a, b) == 0; }
    friend bool operator!=(frac a, frac b) { return compare(a, b) != 0; }
    friend bool operator<(frac a, frac b) { return compare(a, b) < 0; }
    friend bool operator>(frac a, frac b) { return compare(a, b) > 0; }
    friend bool operator<=(frac a, frac b) { return compare(a, b) <= 0; }
    friend bool operator>=(frac a, frac b) { return compare(a, b) >= 0; }

    static int compare(frac a, frac b) {
        if (a.d == 0 || b.d == 0) {
            return infsign(a) - infsign(b);
        } else if constexpr (SAFE) {
            return sign(a.n * b.d - b.n * a.d);
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

    friend frac operator+(frac a, frac b) {
        if (a.d == 0 && b.d == 0) {
            return assert(a.n == b.n), a;
        } else if (a.d == 0 || b.d == 0) {
            return a.d == 0 ? a : b;
        } else if (a.d == b.d) {
            return frac(a.n + b.n, a.d);
        } else {
            return frac(a.n * b.d + b.n * a.d, a.d * b.d);
        }
    }
    friend frac operator-(frac a, frac b) { return a + (-b); }
    friend frac operator*(frac a, frac b) {
        if (a.n == b.d) {
            return frac(b.n, a.d);
        } else if (a.d == b.n) {
            return frac(a.n, b.d);
        } else {
            return frac(a.n * b.n, a.d * b.d);
        }
    }
    friend frac operator/(frac a, frac b) { return a * inv(b); }
    friend frac operator%(frac a, frac b) { return a - T(a / b) * b; }
    friend frac& operator+=(frac& a, frac b) { return a = a + b; }
    friend frac& operator-=(frac& a, frac b) { return a = a - b; }
    friend frac& operator*=(frac& a, frac b) { return a = a * b; }
    friend frac& operator/=(frac& a, frac b) { return a = a / b; }
    friend frac& operator%=(frac& a, frac b) { return a = a % b; }

    friend frac operator-(frac f) { return frac(-f.n, f.d); }
    friend bool operator!(frac f) { return f.n == 0; }

    friend string to_string(frac f) {
        if (f.d == 0) {
            return f.n ? f.n > 0 ? "inf" : "-inf" : "undef";
        } else if (f.d == 1) {
            return to_string(f.n);
        } else {
            return to_string(f.n) + '/' + to_string(f.d);
        }
    }

    friend ostream& operator<<(ostream& out, frac f) { return out << to_string(f); }
};

template <typename Frac>
Frac stofrac(const string& s) {
    int i = 0, S = s.size();
    while (i < S && isspace(s[i]))
        i++; // skip whitespace
    int j = i;
    bool neg = j < S && s[j] == '-', pos = j < S && s[j] == '+';
    j += neg || pos; // skip leading sign
    typename Frac::unit_type integer = 0;
    while (j < S && '0' <= s[j] && s[j] <= '9') {
        integer = 10 * integer + (s[j] - '0'), j++; // read digits
    }
    integer = neg ? -integer : integer;
    if (i < j && j < S && s[j] == '/') {
        int n = 0, k = j + 1;
        typename Frac::unit_type denom = 0;
        while (k < S && '0' <= s[k] && s[k] <= '9') {
            denom = 10 * denom + (s[k] - '0'), n++, k++; // read digits
        }
        return n ? Frac(integer, denom) : Frac(integer);
    } else if (j < S && s[j] == '.') {
        int n = 0, k = j + 1;
        typename Frac::unit_type decimal = 0, ten = 1;
        while (k < S && '0' <= s[k] && s[k] <= '9') {
            decimal = 10 * decimal + (s[k] - '0'), ten *= 10, n++, k++; // read digits
        }
        decimal = neg ? -decimal : decimal;
        return Frac(integer * ten + decimal, ten);
    } else {
        return integer;
    }
}
template <typename T>
istream& operator>>(istream& in, frac<T>& f) {
    string s;
    return in >> s, f = stofrac<frac<T>>(s), in;
}

namespace std {

template <typename T, bool SAFE>
struct hash<frac<T, SAFE>> {
    size_t operator()(const frac<T, SAFE>& f) const noexcept {
        size_t a = hash<T>{}(f.n), b = hash<T>{}(f.d);
        return (a + b) * (a + b + 1) / 2 + b;
    }
};
template <typename T, bool SAFE>
struct numeric_limits<frac<T, SAFE>> {
    static constexpr inline bool is_specialized = true, is_exact = true;
    static constexpr inline bool is_bounded = true, is_signed = true;
    static constexpr inline bool is_integer = false, has_infinity = true;
    static inline auto lowest() { return frac<T, SAFE>(numeric_limits<T>::min(), 1); }
    static inline auto min() { return frac<T, SAFE>(numeric_limits<T>::min(), 1); }
    static inline auto max() { return frac<T, SAFE>(numeric_limits<T>::max(), 1); }
    static inline auto infinity() { return frac<T, SAFE>(1, 0); }
    static inline auto epsilon() { return frac<T, SAFE>(0, 1); }
};

} // namespace std

// Given a predicate P that is ..false..true.., binary search for the smallest fraction f
// such that P(f) is true, the numerator of f is at most N, and the denominator is at most
// D. O(log ND) calls. If inflection point i meets the conditions, this returns i when
// p(i) is true, otherwise it returns the next fraction. Based on kactl
template <typename Pred, typename T = int64_t>
auto frac_bounded_search(Pred&& p, T N, T D) {
    int zero = p(0), right = true, A = true, B = true;
    // If p(0) is true we binary search (-inf,0), and negate what we forward to p.
    auto q = [&](const frac<T>& f) { return zero ? !p(-f) : p(f); };
    frac<T> L(0, 1), R(1, 0);
    while (A || B) {
        T adv = 0, step = 1;
        for (int s = 0; step; s ? step /= 2 : step *= 2) {
            adv += step;
            frac<T> mid(L.n * adv + R.n, L.d * adv + R.d);
            if (mid.n > N || mid.d > D || (right ^ q(mid))) {
                adv -= step, s = 1;
            }
        }
        R.n += L.n * adv;
        R.d += L.d * adv;
        right = !right;
        swap(L, R);
        A = B, B = !!adv;
    }
    auto ans = right ^ zero ? R : L;
    return zero ? -ans : ans;
}

template <typename Pred, typename T = int64_t>
auto frac_bounded_search(Pred&& p, T N, const frac<T>& L, const frac<T>& R) {
    auto q = [&](const frac<T>& f) { return f < L ? false : R < f ? true : p(f); };
    return frac_bounded_search(q, N);
}

// Given a predicate P that is ..false..true.., binary search for the fraction f such that
// P(f) is true, without knowing a bound on its numerator or denominator. Protocol:
// 0 => too small go right, 1 => ok, 2 => too big go left. O(log ND) calls. Based on kactl
template <typename Pred, typename T = int64_t>
auto frac_binary_search(Pred&& p) {
    int zero = p(0), right = 2;
    if (zero == 1) {
        return frac<T>(0);
    }
    // If p(0) is true we binary search (-inf,0), and negate what we forward to p.
    auto q = [&](const frac<T>& f) { return zero ? 2 - p(-f) : p(f); };
    frac<T> L(0, 1), R(1, 0);
    while (true) {
        T adv = 0, step = 1;
        for (int s = 0; step; s ? step /= 2 : step *= 2) {
            adv += step;
            frac<T> mid(L.n * adv + R.n, L.d * adv + R.d);
            if (auto res = q(mid); res == 1) {
                return zero ? -mid : mid;
            } else if (right != res) {
                adv -= step, s = 1;
            }
        }
        R.n += L.n * adv;
        R.d += L.d * adv;
        right = 2 - right;
        swap(L, R);
    }
}

template <typename Pred, typename T = int64_t>
auto frac_binary_search(Pred&& p, const frac<T>& L, const frac<T>& R) {
    auto q = [&](const frac<T>& f) { return f < L ? 0 : R < f ? 2 : p(f); };
    return frac_binary_search(q);
}
