#pragma once

#include "numeric/fft.hpp"
#include "numeric/modnum.hpp"
#include "numeric/combinatorics.hpp"
#include "algo/y_combinator.hpp"

// Core operations, multiplication, fft
namespace polymath {
#define TTT template <typename T>

TTT auto rand_poly(int n) {
    static mt19937 rng(random_device{}());
    static uniform_int_distribution<int> dist(1, T::MOD - 1);
    vector<T> v(n);
    for (int i = 0; i < n; i++) {
        v[i] = T(dist(rng));
    }
    return v;
}

// TODO: Fill in core FFT functions
TTT void fft(vector<T>& u) { fft::fft_transform<0>(u, u.size()); }
TTT void ifft(vector<T>& u) { fft::fft_transform<1>(u, u.size()); }
TTT auto convolve(const vector<T>& a, const vector<T>& b) {
    return fft::ntt_multiply(a, b);
}

// Utility stuff, then elementary operations, then FFT and power series operations

TTT int size(const vector<T>& u) { return u.size(); }
TTT int deg(const vector<T>& u) { return size(u) - 1; }
TTT auto cell(const vector<T>& u, int i) { return i < size(u) ? u[i] : T(); }

TTT auto& reverse(vector<T>& u) { return reverse(begin(u), end(u)), u; }
TTT auto& grow(vector<T>& u, int n) { return u.resize(max(size(u), n)), u; }
TTT auto& resize(vector<T>& u, int n) { return u.resize(n), u; }
TTT auto& shrink(vector<T>& u, int n) { return u.resize(min(size(u), n)), u; }
TTT auto& trim(vector<T>& u, int upto = 0) {
    while (size(u) > upto && u.back() == T(0))
        u.pop_back();
    return u;
}

TTT auto reversed(vector<T> u) { return reverse(u), u; }
TTT auto grown(vector<T> u, int n) { return grow(u, n), u; }
TTT auto resized(vector<T> u, int n) { return resize(u, n), u; }
TTT auto shrunk(vector<T> u, int n) { return shrink(u, n), u; }
TTT auto trimmed(vector<T> u) { return trim(u), u; }

TTT auto& operator+=(vector<T>& u, T value) { return grow(u, 1), u[0] += value, u; }
TTT auto& operator-=(vector<T>& u, T value) { return grow(u, 1), u[0] -= value, u; }
TTT auto& operator*=(vector<T>& u, T value) {
    for (int i = 0; i < size(u); i++)
        u[i] *= value;
    return u;
}
TTT auto& operator/=(vector<T>& u, T value) { return u *= T(1) / value, u; }
TTT auto& operator+=(vector<T>& u, const vector<T>& v) {
    grow(u, size(v));
    for (int i = 0; i < size(v); i++)
        u[i] += v[i];
    return u;
}
TTT auto& operator-=(vector<T>& u, const vector<T>& v) {
    grow(u, size(v));
    for (int i = 0; i < size(v); i++)
        u[i] -= v[i];
    return u;
}
TTT auto& operator*=(vector<T>& u, const vector<T>& v) { return u = convolve(u, v), u; }

TTT auto operator-(vector<T> u) {
    for (int i = 0; i < size(u); i++)
        u[i] = -u[i];
    return u;
}

TTT auto operator+(vector<T> u, T value) { return u += value, u; }
TTT auto operator-(vector<T> u, T value) { return u -= value, u; }
TTT auto operator*(vector<T> u, T value) { return u *= value, u; }
TTT auto operator+(T value, vector<T> u) { return u += value, u; }
TTT auto operator-(T value, vector<T> u) { return u -= value, u; }
TTT auto operator*(T value, vector<T> u) { return u *= value, u; }
TTT auto operator/(vector<T> u, T value) { return u /= value, u; }
TTT auto operator+(vector<T> u, const vector<T>& v) { return u += v, u; }
TTT auto operator-(vector<T> u, const vector<T>& v) { return u -= v, u; }
TTT auto operator*(const vector<T>& u, const vector<T>& v) { return convolve(u, v); }

TTT auto& pointwise_inplace(vector<T>& u, const vector<T>& o) {
    grow(u, size(o));
    for (int i = 0; i < size(o); i++)
        u[i] *= o[i];
    return u;
}
TTT auto pointwise(vector<T> u, const vector<T>& o) { return pointwise_inplace(u, o), u; }

TTT auto inner_product(const vector<T>& u, const vector<T>& v) {
    T sum = 0;
    for (int i = 0, n = min(size(u), size(v)); i < n; i++)
        sum += u[i] * v[i];
    return sum;
}

TTT auto& rem_shift(vector<T>& u, int s) {
    return u.erase(begin(u), begin(u) + min(size(u), s)), u;
}
TTT auto& add_shift(vector<T>& u, int s, T add = T(0)) {
    return u.insert(begin(u), s, add), u;
}

TTT auto& operator>>=(vector<T>& u, int s) { return rem_shift(u, s); }
TTT auto& operator<<=(vector<T>& u, int s) { return add_shift(u, s); }
TTT auto operator>>(vector<T> u, int s) { return u >>= s, u; }
TTT auto operator<<(vector<T> u, int s) { return u <<= s, u; }

TTT auto& derivative_inplace(vector<T>& u) {
    for (int i = 1; i < size(u); i++)
        u[i - 1] = T(i) * u[i];
    return size(u) > 0 ? u.pop_back(), u : u;
}
TTT auto derivative(vector<T> u) { return derivative_inplace(u), u; }

TTT auto& integral_inplace(vector<T>& u, T zero = 0) {
    u.push_back(T(0));
    for (int i = size(u) - 1; i > 0; i--)
        u[i] = u[i - 1] / T(i);
    return u[0] = zero, u;
}
TTT auto integral(vector<T> u) { return integral_inplace(u), u; }

TTT auto eval(const vector<T>& u, T x) {
    T sum = 0;
    for (int i = size(u) - 1; i >= 0; i--)
        sum = u[i] + sum * x;
    return sum;
}

} // namespace polymath

// Series inverse, exp, log, pow, division, composition
namespace polymath {

// Compute first n terms of series 1/u. O(n log n)
TTT auto inverse(const vector<T>& u, int n = -1) {
    assert(size(u) > 0 && u[0] != T(0));
    n = n < 0 ? size(u) : n;

    vector<T> ans(n, T(1) / u[0]);

    for (int d = 1; d < n; d *= 2) {
        vector<T> f(2 * d), g(2 * d);

        for (int j = 0; j < min(2 * d, size(u)); j++)
            f[j] = u[j];
        for (int j = 0; j < d; j++)
            g[j] = ans[j];

        fft(f), fft(g);
        for (int j = 0; j < 2 * d; j++)
            f[j] *= g[j];

        ifft(f);
        for (int j = 0; j < d; j++)
            f[j] = 0;

        fft(f);
        for (int j = 0; j < 2 * d; j++)
            f[j] *= g[j];

        ifft(f);
        for (int j = d; j < min(2 * d, n); j++)
            ans[j] = -f[j];
    }

    return ans;
}

// Compute first n terms of series exp(u). O(n log n)
TTT auto exp(const vector<T>& u, int n = -1) {
    assert(size(u) == 0 || u[0] == T(0));
    n = n < 0 ? size(u) : n;

    vector<T> b{1, size(u) > 1 ? u[1] : 0}, c{1}, z1, z2{1, 1};
    b.reserve(n);

    for (int m = 2; m < n; m *= 2) {
        auto y = b;
        resize(y, 2 * m);
        fft(y);

        z1 = move(z2);
        vector<T> z(m);
        for (int i = 0; i < m; ++i)
            z[i] = y[i] * z1[i];
        ifft(z);
        fill(begin(z), begin(z) + m / 2, T(0));
        fft(z);
        for (int i = 0; i < m; ++i)
            z[i] *= -z1[i];
        ifft(z);

        c.insert(end(c), begin(z) + m / 2, end(z));
        z2 = c;
        resize(z2, 2 * m);
        fft(z2);

        vector<T> x(begin(u), begin(u) + min(size(u), m));
        resize(x, m);
        derivative_inplace(x);
        x.push_back(T(0));

        fft(x);
        for (int i = 0; i < m; ++i)
            x[i] *= y[i];
        ifft(x);

        x -= derivative(b);
        resize(x, 2 * m);
        for (int i = 0; i < m - 1; ++i)
            x[m + i] = x[i], x[i] = T(0);

        fft(x);
        for (int i = 0; i < 2 * m; ++i)
            x[i] *= z2[i];
        ifft(x);

        x.pop_back();
        integral_inplace(x);
        for (int i = m; i < min(size(u), 2 * m); ++i)
            x[i] += u[i];
        fill(begin(x), begin(x) + m, T(0));

        fft(x);
        for (int i = 0; i < 2 * m; ++i)
            x[i] *= y[i];
        ifft(x);

        b.insert(end(b), begin(x) + m, end(x));
    }

    resize(b, n);
    return b;
}

// Compute first n terms of series log(u). O(n log n)
TTT auto log(const vector<T>& u, int n = -1) {
    assert(size(u) > 0 && u[0] == 1);
    n = n < 0 ? size(u) : n;
    auto v = derivative(u) * inverse(u, n);
    resize(integral_inplace(v), n - 1);
    return v;
}

// Compute first n terms of u^k. O(n log n)
TTT auto pow(const vector<T>& u, int64_t k, int n = -1) {
    n = n < 0 ? size(u) : n;
    if (k == 0) {
        return resized(vector<T>{1}, n);
    } else if (k == 1) {
        return resized(u, n);
    } else if (k == 2) {
        return resized(u * u, n);
    } else if (k < 0) {
        return inverse(pow(u, -k, n));
    }

    for (int i = 0; i < size(u) && i * k < n; i++) {
        if (u[i] != 0) {
            auto ans = exp(T(k) * log((u >> i) / u[i], n)) * modpow(u[i], k);
            resize(ans <<= i * k, n);
            return ans;
        }
    }

    return vector<T>(n, 0);
}

// Compute quotient u/v as proper polynomial division. O(n log n)
TTT auto operator/(vector<T> u, vector<T> v) {
    trim(u), trim(v);
    assert(size(v) > 0);

    if (size(u) < size(v)) {
        return vector<T>();
    } else {
        int n = size(u) - size(v) + 1;
        reverse(u), reverse(v);
        auto q = u * inverse(v, n);
        shrink(q, n), reverse(q), trim(q);
        return q;
    }
}

// Compute remainder u%v as proper polynomial division. O(n log n)
TTT auto operator%(vector<T> u, vector<T> v) { return u -= v * (u / v), trim(u), u; }

TTT auto& operator/=(vector<T>& u, vector<T> v) { return u = u / move(v), u; }
TTT auto& operator%=(vector<T>& u, vector<T> v) { return u = u % move(v), u; }

// Compute quotient and remainder of u/v as proper polynomial division. O(n log n)
TTT auto divrem(vector<T> u, vector<T> v) {
    auto q = u / v;
    u -= q * v, trim(u);
    return make_pair(move(q), move(u));
}

// Get k-th term of linear recurrence with coefs. rec and initial values x. O(n log n)
TTT auto kitamasa(const vector<T>& x, const vector<T>& rec, int64_t k) {
    // Source: Nyaan https://judge.yosupo.jp/submission/36598
    if (k < size(x)) {
        return x[k];
    }
    auto Q = -rec;
    trim(Q), Q <<= 1, Q[0] = 1;
    int rec_len = size(Q) - 1;
    auto P = shrunk(x, rec_len) * Q;
    resize(P, rec_len);

    T ret = 0;
    int N = 1;
    while (N < size(Q)) {
        N <<= 1;
    }

    P.resize(2 * N);
    Q.resize(2 * N);
    fft(P), fft(Q);

    T dw = T(1) / fft::root_of_unity<T>::get(2 * N);
    vector<T> A(2 * N), B(2 * N);
    const int* bits = fft::fft_reverse_cache::get(N);

    while (k > 0) {
        T inv2 = T(1) / T(2);

        B.resize(N);
        for (int j = 0; j < N; j++) {
            int i = bits[j], l = i << 1, r = i << 1 | 1;
            B[i] = Q[l] * Q[r];
        }

        A.resize(N);
        if (k & 1) {
            // odd degree of P(x)Q(-x)
            for (int j = 0; j < N; j++) {
                int i = bits[j], l = i << 1, r = i << 1 | 1;
                A[i] = (P[l] * Q[r] - P[r] * Q[l]) * inv2;
                inv2 *= dw;
            }
        } else {
            // even degree of P(x)Q(-x)
            for (int i = 0; i < N; i++) {
                int l = i << 1, r = i << 1 | 1;
                A[i] = (P[l] * Q[r] + P[r] * Q[l]) * inv2;
            }
        }
        swap(P, A);
        swap(Q, B);
        k >>= 1;
        if (k < N) {
            break;
        }
        fft::fft_doubling_inplace(P, N);
        fft::fft_doubling_inplace(Q, N);
    }
    ifft(P), ifft(Q);
    return ret + (P * inverse(Q))[k];
}

// Composition p(q(x)), naively quadratic, complexity O(n² log n)
TTT auto naive_composition(const vector<T>& p, vector<T> q, int n = -1) {
    n = n < 0 ? (size(p) - 1) * (size(q) - 1) + 1 : n;
    if (n == 0) {
        return vector<T>();
    }

    trim(q);
    vector<T> ans(n), u{1};
    u.reserve(n);

    for (int i = 0; i < min(n, size(p)); i++) {
        ans += p[i] * u, u = shrunk(u * q, n);
    }

    return ans;
}

// Composition p(q(x)), square-root decomposition, complexity O((n log n)^3/2)
TTT auto sqrt_composition(const vector<T>& p, vector<T> q, int n = -1) {
    // Source: https://judge.yosupo.jp/submission/43112
    n = n < 0 ? (size(p) - 1) * (size(q) - 1) + 1 : n;
    if (n == 0) {
        return vector<T>();
    }

    trim(q);
    int k = sqrt(n);
    int d = n / k + (k * k < n);

    vector<vector<T>> small(d + 1);
    small[0] = {1}, small[1] = shrunk(q, n);
    for (int i = 2; i <= d; i++) {
        small[i] = small[i / 2] * small[(i + 1) / 2];
        shrink(small[i], n);
    }

    vector<vector<T>> fi(k);
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < d && i * d + j < min(n, size(p)); j++) {
            int x = i * d + j;
            fi[i] += p[x] * small[j];
        }
    }

    vector<T> ans(n), big = {1};
    for (int i = 0; i < k; i++) {
        fi[i] = shrunk(fi[i] * big, n);
        ans += fi[i];
        big = shrunk(big * small[d], n);
    }
    return ans;
}

TTT auto composition(const vector<T>& u, const vector<T>& v, int n = -1) {
    return sqrt_composition(u, v, n);
}

} // namespace polymath

// Polynomial gcd, resultant
namespace polymath {

TTT struct polymat {
    bool identity = false;
    vector<T> a0, a1, b0, b1;

    polymat() : identity(true) {}
    explicit polymat(vector<T> v) : a1({1}), b0({1}), b1(move(v)) {}

    polymat(vector<T> a0, vector<T> a1, vector<T> b0, vector<T> b1)
        : identity(false), a0(move(a0)), a1(move(a1)), b0(move(b0)), b1(move(b1)) {}

    friend polymat operator*(const polymat& lhs, const polymat& rhs) {
        if (lhs.identity || rhs.identity)
            return lhs.identity ? rhs : lhs;
        return polymat{
            trimmed(lhs.a0 * rhs.a0 + lhs.a1 * rhs.b0), //
            trimmed(lhs.a0 * rhs.a1 + lhs.a1 * rhs.b1), //
            trimmed(lhs.b0 * rhs.a0 + lhs.b1 * rhs.b0), //
            trimmed(lhs.b0 * rhs.a1 + lhs.b1 * rhs.b1), //
        };
    }

    auto multiply(const vector<T>& a, const vector<T>& b) {
        if (identity)
            return make_pair(a, b);
        return make_pair(trimmed(a0 * a + a1 * b), trimmed(b0 * a + b1 * b));
    }
};

TTT auto halfgcd(const vector<T>& p0, const vector<T>& p1) {
    int m = size(p0) >> 1;
    if (size(p1) <= m) {
        return polymat<T>();
    }

    polymat<T> R = halfgcd(p0 >> m, p1 >> m);
    auto [a, b] = R.multiply(p0, p1);
    if (size(b) <= m) {
        return R;
    }

    auto [q, r] = divrem(a, b);

    polymat<T> Q(-q);
    int k = (m << 1) - size(b) + 1;
    if (size(r) <= k) {
        return Q * R;
    }

    return halfgcd(b >> k, r >> k) * Q * R;
}

TTT auto cogcd(const vector<T>& p0, const vector<T>& p1) {
    polymat<T> M = halfgcd(p0, p1);
    auto [a, b] = M.multiply(p0, p1);
    if (size(b) == 0) {
        return M;
    }

    auto [q, r] = divrem(a, b);
    polymat<T> Q(-q);
    if (size(r) == 0) {
        return Q * M;
    }

    return cogcd(b, r) * Q * M;
}

TTT auto exgcd(vector<T> a, vector<T> b, vector<T>& x, vector<T>& y) {
    // ax + by = monic gcd(a,b)
    trim(a), trim(b);
    if (size(a) == 0 || size(b) == 0) {
        x = y = {1};
        return size(b) == 0 ? a : b;
    }

    if (size(a) > size(b)) {
        auto c = cogcd(a, b);
        x = move(c.a0), y = move(c.a1);
    } else if (size(a) < size(b)) {
        auto c = cogcd(b, a);
        x = move(c.a1), y = move(c.a0);
    } else {
        auto [q, r] = divrem(a, b);
        auto c = cogcd(b, r) * polymat<T>(-q);
        x = move(c.a0), y = move(c.a1);
    }
    auto g = trimmed(a * x + b * y);

    if (auto v = g.back(); v != T(1)) {
        x /= v, y /= v, g /= v;
    }
    return g;
}

TTT auto gcd(vector<T> a, vector<T> b) {
    while (size(a) > 0) {
        b %= a;
        swap(a, b);
    }
    return size(b) == 0 ? b : b / b.back();
}

TTT auto naive_gcd(vector<T> a, vector<T> b, vector<T>& x, vector<T>& y) {
    if (size(a) == 0 && size(b) == 0) {
        x = y = {};
        return vector<T>();
    }
    vector<T> xn = {1}, yn = {};
    x = {}, y = {1};
    while (size(a) > 0) {
        auto [q, r] = divrem(b, a);
        b = move(r);
        x = x - q * xn, trim(x);
        y = y - q * yn, trim(y);
        swap(a, b);
        swap(x, xn);
        swap(y, yn);
    }
    if (auto v = b.back(); v != T(1)) {
        x /= v, y /= v, b /= v;
    }
    return b;
}

TTT auto resultant(const vector<T>& a, const vector<T>& b) {
    int A = size(a), B = size(b);
    if (B == 0) {
        return T();
    } else if (B == 1) {
        return modpow(b[0], A - 1);
    } else {
        auto c = a % b;
        A -= size(c);
        int sign = ((A - 1) & (B - 1) & 1) ? -1 : 1;
        auto mul = modpow(b[0], A - 1) * sign;
        return mul * resultant(b, c);
    }
}

} // namespace polymath

// Multipoint evaluation, interpolation
namespace polymath {

constexpr int MULTIEVAL_THRESHOLD = 300;

TTT auto multieval(const vector<T>& p, const vector<T>& x) {
    // Source: https://github.com/Aeren1564/Algorithms
    int S = size(x);
    if (size(p) <= MULTIEVAL_THRESHOLD || size(x) <= MULTIEVAL_THRESHOLD) {
        vector<T> ans(S);
        for (int i = 0; i < S; i++) {
            ans[i] = eval(p, x[i]);
        }
        return ans;
    }

    vector<vector<T>> st(2 * S);

    y_combinator([&](auto self, int u, int l, int r) -> void {
        if (r - l == 1) {
            st[u] = {-x[l], 1};
        } else {
            int m = l + ((r - l) >> 1), v = u + ((m - l) << 1);
            self(u + 1, l, m), self(v, m, r);
            st[u] = st[u + 1] * st[v];
        }
    })(0, 0, S);

    vector<T> ans(S);

    y_combinator([&](auto self, int u, int l, int r, vector<T> f) -> void {
        f %= st[u];
        if (size(f) <= MULTIEVAL_THRESHOLD) {
            for (int i = l; i < r; i++) {
                ans[i] = eval(f, x[i]);
            }
        } else if (r - l == 1) {
            ans[l] = f[0];
        } else {
            int m = l + ((r - l) >> 1), v = u + ((m - l) << 1);
            self(u + 1, l, m, f), self(v, m, r, move(f));
        }
    })(0, 0, S, p);

    return ans;
}

TTT auto interpolate(const vector<T>& x, const vector<T>& y) {
    // Source: https://github.com/Aeren1564/Algorithms
    assert(size(x) == size(y));
    int S = size(x);
    if (S == 0) {
        return vector<T>();
    }

    vector<vector<T>> st(2 * S);

    y_combinator([&](auto self, int u, int l, int r) -> void {
        if (r - l == 1) {
            st[u] = {-x[l], 1};
        } else {
            int m = l + ((r - l) >> 1), v = u + ((m - l) << 1);
            self(u + 1, l, m), self(v, m, r);
            st[u] = st[u + 1] * st[v];
        }
    })(0, 0, S);

    vector<T> val(S);

    y_combinator([&](auto self, int u, int l, int r, vector<T> f) -> void {
        f %= st[u];
        if (size(f) <= MULTIEVAL_THRESHOLD) {
            for (int i = l; i < r; i++) {
                val[i] = eval(f, x[i]);
            }
        } else if (r - l == 1) {
            val[l] = f[0];
        } else {
            int m = l + ((r - l) >> 1), v = u + ((m - l) << 1);
            self(u + 1, l, m, f), self(v, m, r, move(f));
        }
    })(0, 0, S, derivative(st[0]));

    for (int i = 0; i < S; i++) {
        val[i] = y[i] / val[i];
    }

    return y_combinator([&](auto self, int u, int l, int r) -> vector<T> {
        if (r - l == 1) {
            return vector<T>{val[l]};
        } else {
            int m = l + ((r - l) >> 1), v = u + ((m - l) << 1);
            return self(u + 1, l, m) * st[v] + self(v, m, r) * st[u + 1];
        }
    })(0, 0, S);
}

} // namespace polymath

// Famous series, taylor shift. Functions return polynomial with degree n (i.e. size n+1)
namespace polymath::series {

TTT auto convert_to_ogf(vector<T> egf) {
    for (int i = 0, n = egf.size(); i < n; i++) {
        egf[i] *= Binomial<T>::fac(i);
    }
    return egf;
}

TTT auto convert_to_egf(vector<T> ogf) {
    for (int i = 0, n = ogf.size(); i < n; i++) {
        ogf[i] *= Binomial<T>::invfac(i);
    }
    return ogf;
}

TTT auto alternating(vector<T> series, int first_negative) {
    for (int i = first_negative, n = size(series); i < n; i += 2) {
        series[i] = -series[i];
    }
    return series;
}

// (x-x0)(x-x1)(x-x2)... Time: O(n log² n)
TTT auto with_roots(const vector<T>& roots) {
    int n = roots.size();
    if (n == 0) {
        return vector<T>{1};
    }

    queue<vector<T>> Q;

    for (int i = 0; i < n; i++) {
        Q.push({-roots[i], 1});
    }
    while (Q.size() > 1u) {
        auto u = move(Q.front());
        Q.pop();
        auto v = move(Q.front());
        Q.pop();
        Q.push(u * v);
    }

    return move(Q.front());
}

// Find p(x+a). Time: O(n log n)
TTT auto taylor_shift(vector<T> p, T a) {
    int n = size(p);
    p = convert_to_ogf(move(p));
    reverse(p);
    vector<T> g(n, 1);
    for (int i = 1; i < n; i++) {
        g[i] = g[i - 1] * a * Binomial<T>::inv(i);
    }
    p = p * g;
    shrink(p, n);
    reverse(p);
    return convert_to_egf(move(p));
}

// e^x = SUM x^k/k!
TTT auto exponential(int n) {
    vector<T> etox(n + 1);
    for (int i = n; i >= 0; i--) {
        etox[i] = Binomial<T>::invfac(i);
    }
    return etox;
}

// SUM x^k/k
TTT auto harmonic(int n, T zero = 0) {
    vector<T> ans(n + 1);
    ans[0] = zero;
    for (int i = 1; i <= n; i++) {
        ans[i] = Binomial<T>::inv(i);
    }
    return ans;
}

// SUM k^m x^k
TTT auto mth_powers(int n, int m) {
    vector<T> ans(n + 1);
    for (int i = 0; i <= n; i++) {
        ans[i] = modpow(T(i), m);
    }
    return ans;
}

// x(x+1)(x+2)...(x+n-1). Time: O(n log n)
TTT auto rising_factorial(int n) {
    if (n == 0) {
        return vector<T>{1};
    }
    int lg = 31 - __builtin_clz(n);
    vector<T> f = {0, 1};
    for (int i = lg - 1; i >= 0; i--) {
        int k = n >> i;
        f *= taylor_shift(f, T(k >> 1));
        if (k & 1) {
            f.push_back(T(0));
            for (int j = size(f) - 1; j > 0; j--) {
                f[j] = f[j - 1] + T(k - 1) * f[j];
            } // f = (f << 1) + (k - 1)f
        }
    }
    return f;
}

// x(x-1)(x-2)...(x-n+1). Time: O(n log n)
TTT auto falling_factorial(int n) {
    return alternating(rising_factorial<T>(n), 1 + n % 2);
}

// Faulhaber on exponent: faul[i] = 1^i + 2^i + ... + m^i. Time: O(n log n)
TTT auto faulhaber(int n, int m) {
    n += 1;              // include n
    vector<T> ex(n + 1); // e^x = SUM x^k/k!
    for (int i = n; i >= 0; i--) {
        ex[i] = Binomial<T>::invfac(i);
    }
    vector<T> den(n);
    for (int i = n - 1; i >= 0; i--) {
        den[i] = -ex[i + 1];
    }
    vector<T> num(n);
    T p = 1;
    for (int i = 0; i < n; i++) {
        p *= m + 1;
        num[i] = ex[i + 1] * (1 - p);
    }
    return convert_to_ogf(resized(num * inverse(den, n), n));
}

// Unsigned Stirling 1st kind, fixed n. Time: O(n log n)
TTT auto stirling_1st(int n) { return rising_factorial<T>(n); }

// Unsigned Stirling 1st kind, fixed k. Time: O(n log n)
TTT auto stirling_1st(int n, int k) {
    return convert_to_ogf(pow(harmonic<T>(n), k) * Binomial<T>::invfac(k));
}

// Stirling 2nd kind, fixed n. Time: O(n log n)
TTT auto stirling_2nd(int n) {
    auto a = alternating(exponential<T>(n), 1);
    auto b = convert_to_egf(mth_powers<T>(n, n));
    return resized(a * b, n + 1);
}

// Stirling 2nd kind, fixed k. Time: O(n log n)
TTT auto stirling_2nd(int n, int k) {
    return convert_to_ogf(pow(exponential<T>(n) - T(1), k) * Binomial<T>::invfac(k));
}

// Bell numbers: number of partitions of a set of n elements. Time: O(n log n)
TTT auto bell(int n) { return convert_to_ogf(exp(exponential<T>(n) - T(1))); }

// Partition number: number of unrestricted partitions of integer n. Time: O(n log n)
TTT auto partitions(int n) {
    vector<T> p(n + 1);
    for (int k = 0, t = 1; k * (3 * k - 1) / 2 <= n; k++, t = -t) {
        p[k * (3 * k - 1) / 2] = t;
    }
    for (int k = 1, t = -1; k * (3 * k + 1) / 2 <= n; k++, t = -t) {
        p[k * (3 * k + 1) / 2] = t;
    }
    return inverse(p, n + 1);
}

// Bernoulli numbers OGF. Time: O(n log n)
TTT auto bernoulli(int n) {
    return convert_to_ogf(inverse((exponential<T>(n + 2) >> 1)));
}

// Eulerian numbers, fixed n. eulerian[k] = A(n,k). Time: O(n log n)
TTT auto eulerian(int n) {
    return resized(mth_powers<T>(n, n) * pow<T>({1, -1}, n + 1, n + 1), n + 1);
}

// TODO: Eulerian OGF for fixed k?

TTT auto sine(int n) {
    vector<T> ans(n + 1);
    for (int i = 1; i <= n; i += 2) {
        ans[i] = (i & 2) ? -Binomial<T>::invfac(i) : Binomial<T>::invfac(i);
    }
    return ans;
}

TTT auto cosine(int n) {
    vector<T> ans(n + 1);
    for (int i = 0; i <= n; i += 2) {
        ans[i] = (i & 2) ? -Binomial<T>::invfac(i) : Binomial<T>::invfac(i);
    }
    return ans;
}

TTT auto secant(int n) { return inverse(cosine<T>(n)); }

TTT auto tangent(int n) { return resized(sine<T>(n) * secant<T>(n), n + 1); }

// Euler number E(n): number of alternating permutations of size n. Time: O(n log n)
TTT auto euler_alternating(int n) {
    auto v = secant<T>(n) + tangent<T>(n);
    return convert_to_ogf(resized(v * v, n + 1));
}

} // namespace polymath::series
