#pragma once

#include "numeric/modnum.hpp"

// Base include with my_complex, reverse/root/scratch caches, naive mult, fft_transform
namespace fft {

template <typename T>
struct my_complex {
    using self = my_complex<T>;
    T x, y;
    constexpr my_complex(T x = T(0), T y = T(0)) : x(x), y(y) {}

    constexpr T& real() { return x; }
    constexpr T& imag() { return y; }
    constexpr const T& real() const { return x; }
    constexpr const T& imag() const { return y; }
    constexpr void real(T v) { x = v; }
    constexpr void imag(T v) { y = v; }
    constexpr friend auto real(self a) { return a.x; }
    constexpr friend auto imag(self a) { return a.y; }
    constexpr self rot_ccw(self a) { return self(-a.y, a.x); }
    constexpr self rot_cw(self a) { return self(a.y, -a.x); }
    constexpr friend auto abs(self a) { return sqrt(norm(a)); }
    constexpr friend auto arg(self a) { return atan2(a.y, a.x); }
    constexpr friend auto norm(self a) { return a.x * a.x + a.y * a.y; }
    constexpr friend auto conj(self a) { return self(a.x, -a.y); }
    constexpr friend auto inv(self a) { return self(a.x / norm(a), -a.y / norm(a)); }
    constexpr friend auto polar(T r, T theta = T()) {
        return self(r * cos(theta), r * sin(theta));
    }
    constexpr T& operator[](int i) { assert(i == 0 || i == 1), *(&x + i); }
    constexpr const T& operator[](int i) const { assert(i == 0 || i == 1), *(&x + i); }

    constexpr self& operator+=(self b) { return *this = *this + b; }
    constexpr self& operator-=(self b) { return *this = *this - b; }
    constexpr self& operator*=(self b) { return *this = *this * b; }
    constexpr self& operator/=(self b) { return *this = *this / b; }

    constexpr friend self operator*(self a, T b) { return self(a.x * b, a.y * b); }
    constexpr friend self operator*(T a, self b) { return self(a * b.x, a * b.y); }
    constexpr friend self operator/(self a, T b) { return self(a.x / b, a.y / b); }
    constexpr friend self operator/(T a, self b) { return a * inv(b); }
    constexpr friend self operator+(self a) { return self(a.x, a.y); }
    constexpr friend self operator-(self a) { return self(-a.x, -a.y); }
    constexpr friend self operator+(self a, self b) { return self(a.x + b.x, a.y + b.y); }
    constexpr friend self operator-(self a, self b) { return self(a.x - b.x, a.y - b.y); }
    constexpr friend self operator/(self a, self b) { return a * inv(b); }
    constexpr friend self operator*(self a, self b) {
        return self(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
    }
};

using default_complex = my_complex<double>;
constexpr double TAU = 6.283185307179586476925286766559;

int next_two(int32_t N) { return N > 1 ? 8 * sizeof(N) - __builtin_clz(N - 1) : 0; }

template <typename T, typename D>
inline constexpr T fft_round(D coefficient) {
    return is_integral<T>::value ? llround(coefficient) : coefficient;
}

template <typename T>
void trim_vector(vector<T>& v) {
    if constexpr (is_floating_point<T>::value)
        while (!v.empty() && abs(v.back()) <= 10 * numeric_limits<T>::epsilon())
            v.pop_back();
    else
        while (!v.empty() && v.back() == T(0))
            v.pop_back();
}

template <typename T>
auto naive_multiply(const vector<T>& a, const vector<T>& b) {
    int A = a.size(), B = b.size(), C = A && B ? A + B - 1 : 0;
    vector<T> c(C);
    for (int i = 0; i < A && B; i++)
        for (int j = 0; j < B; j++)
            c[i + j] += a[i] * b[j];
    trim_vector(c);
    return c;
}

struct fft_reverse_cache {
    static inline vector<vector<int>> rev;

    static const int* get(int N) {
        int n = next_two(N), r = rev.size();
        if (n >= r || rev[n].empty()) {
            rev.resize(max(r, n + 1));
            int R = 1 << n;
            rev[n].assign(R, 0);
            for (int i = 0; i < R; i++) {
                rev[n][i] = (rev[n][i >> 1] | ((i & 1) << n)) >> 1;
            }
        }
        return rev[n].data();
    }
};

template <typename T>
struct root_of_unity {};

template <typename D>
struct root_of_unity<my_complex<D>> {
    static auto get(int n) {
        assert(n > 0);
        return my_complex<D>(cos(TAU / n), sin(TAU / n));
    }
};

template <typename C>
struct fft_roots_cache {
    static inline vector<C> root = vector<C>(2, C(1));
    static inline vector<C> invroot = vector<C>(2, C(1));
    static inline vector<C> scratch_a, scratch_b;

    static auto get(int N) {
        for (int k = root.size(); k < N; k *= 2) {
            root.resize(2 * k);
            invroot.resize(2 * k);
            auto z = root_of_unity<C>::get(2 * k);
            auto iz = C(1) / z;
            for (int i = k / 2; i < k; i++) {
                root[2 * i] = root[i];
                root[2 * i + 1] = root[i] * z;
                invroot[2 * i] = invroot[i];
                invroot[2 * i + 1] = invroot[i] * iz;
            }
        }
        return make_pair(root.data(), invroot.data());
    }

    static auto get_scratch(int N) {
        if (int(scratch_a.size()) < N) {
            scratch_a.resize(N);
            scratch_b.resize(N);
        }
        return make_pair(ref(scratch_a), ref(scratch_b));
    }
};

template <typename T>
void fft_bit_reverse(vector<T>& a, int N) {
    auto rev = fft_reverse_cache::get(N);
    for (int i = 0; i < N; i++) {
        if (i < rev[i]) {
            swap(a[i], a[rev[i]]);
        }
    }
}

template <bool inverse, bool standard = true, typename T>
void fft_transform(vector<T>& a, int N) {
    // With standard=true, reverse bits at start and end of forward transform.
    // With standard=false, reverse bits at start of forward and backward transform.
    if constexpr (!standard || !inverse) {
        fft_bit_reverse(a, N);
    }
    auto [root, invroot] = fft_roots_cache<T>::get(N);
    for (int k = 1; k < N; k *= 2) {
        for (int i = 0; i < N; i += 2 * k) {
            for (int l = i, r = i + k, j = 0; j < k; j++, l++, r++) {
                auto z = inverse ? invroot[j + k] : root[j + k];
                auto t = a[r] * z;
                a[r] = a[l] - t;
                a[l] = a[l] + t;
            }
        }
    }
    if constexpr (inverse) {
        auto inv = T(1) / T(N);
        for (int i = 0; i < N; i++) {
            a[i] *= inv;
        }
    } else if constexpr (standard) {
        fft_bit_reverse(a, N);
    }
}

template <typename T> // a is fft'd and not bit-reversed, extend it to more roots
void fft_doubling_inplace(vector<T>& a, int N) {
    auto fb = fft_roots_cache<T>::get_scratch(N).second;
    for (int i = 0; i < N; i++) {
        fb[i] = a[i];
    }
    fft_transform<1>(fb, N);
    T r = 1, zeta = root_of_unity<T>::get(2 * N);
    for (int i = 0; i < N; i++) {
        fb[i] *= r, r *= zeta;
    }
    fft_transform<0>(fb, N);
    a.resize(max(int(a.size()), 2 * N));
    for (int i = 0; i < N; i++) {
        a[N + i] = fb[i];
    }
}

} // namespace fft

// NTT with modnums | 60,125,270ms for 250K,500K,1M
namespace fft {

template <uint32_t MOD>
struct root_of_unity<modnum<MOD>> {
    using type = modnum<MOD>;
    static constexpr int ntt_primitive_root(int p) {
        if (p == 2 || p == 3)
            return p - 1;
        if (p == 167772161 || p == 469762049 || p == 998244353)
            return 3;
        if (p == 1000000007)
            return 5;
        assert(p % 2 == 1);

        auto modpow = [](int base, int64_t e, int mod) {
            int x = 1;
            base = base % mod;
            while (e > 0) {
                if (e & 1)
                    x = int64_t(x) * int64_t(base) % mod;
                if (e >>= 1)
                    base = int64_t(base) * int64_t(base) % mod;
            }
            return x;
        };

        int divs[20] = {2};
        int cnt = 1;
        int x = (p - 1) / 2; // phi(p)
        while (x % 2 == 0) {
            x /= 2;
        }
        for (int i = 3; i * i <= x; i += 2) {
            if (x % i == 0) {
                divs[cnt++] = i;
                while (x % i == 0) {
                    x /= i;
                }
            }
        }
        if (x > 1) {
            divs[cnt++] = x;
        }
        for (int g = 2;; g++) {
            bool ok = true;
            for (int i = 0; i < cnt; i++) {
                if (modpow(g, (p - 1) / divs[i], p) == 1) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                return g;
            }
        }
        __builtin_unreachable();
    }
    static constexpr type get(int n) {
        modnum<MOD> g = ntt_primitive_root(MOD);
        assert(n > 0 && (MOD - 1) % n == 0 && "Modulus cannot handle NTT this large");
        return modpow(g, int64_t(MOD - 1) / n);
    }
};

template <uint32_t MOD>
auto ntt_multiply(const vector<modnum<MOD>>& a, const vector<modnum<MOD>>& b) {
    using T = modnum<MOD>;
    if (a.empty() || b.empty()) {
        return vector<T>();
    }
    int A = a.size(), B = b.size();
    int S = A + B - 1, s = next_two(S), N = 1 << s;
    if (min(A, B) <= 5 || 1.0 * A * B <= 2.5 * N * s) {
        return naive_multiply(a, b);
    }

    auto fb = fft_roots_cache<T>::get_scratch(N).second;
    vector<T> c(N);
    copy_n(begin(a), A, begin(c));
    copy_n(begin(b), B, begin(fb));
    fill_n(begin(fb) + B, N - B, T(0));
    fft_transform<0>(c, N);
    fft_transform<0>(fb, N);
    for (int i = 0; i < N; i++) {
        c[i] = c[i] * fb[i];
    }
    fft_transform<1>(c, N);
    trim_vector(c);
    return c;
}

} // namespace fft

// FFT with complex numbers (for int, long, double) | 50,110,230ms for 250K,500K,1M
namespace fft {

template <typename C = default_complex, typename T>
auto fft_multiply(const vector<T>& a, const vector<T>& b) {
    if (a.empty() || b.empty()) {
        return vector<T>();
    }
    int A = a.size(), B = b.size();
    int S = A + B - 1, s = next_two(S), N = 1 << s;
    if (min(A, B) <= 5 || 1.0 * A * B <= 8.0 * N * s) {
        return naive_multiply(a, b);
    }

    auto [fa, fb] = fft_roots_cache<C>::get_scratch(N);
    for (int i = 0; i < min(A, B); i++) {
        fa[i] = C(a[i], b[i]);
    }
    for (int i = B; i < A; i++) {
        fa[i] = C(a[i], 0);
    }
    for (int i = A; i < B; i++) {
        fa[i] = C(0, b[i]);
    }
    fill_n(begin(fa) + max(A, B), N - max(A, B), C(0));
    fft_transform<0, 0>(fa, N);
    for (int i = 0, j = 0; i < N; i++, j = N - i) {
        fb[i] = (fa[i] * fa[i] - conj(fa[j] * fa[j])) * C(0, -0.25);
    }
    fft_transform<1, 0>(fb, N);
    vector<T> c(S);
    for (int i = 0; i < S; i++) {
        c[i] = fft_round<T>(fb[i].real());
    }
    trim_vector(c);
    return c;
}

} // namespace fft

// FFT-SPLIT (for large ints, longs, doubles) | 120,270,560ms for 250K,500K,1M
namespace fft {

template <typename T>
auto maxabsolute(const vector<T>& a) {
    T maxabs = T(0);
    for (const T& n : a)
        maxabs = max(maxabs, abs(n));
    return maxabs;
}

template <typename T, typename C>
void fft_split_lower_upper(T d, const vector<T>& a, vector<C>& comp) {
    for (int i = 0, A = a.size(); i < A; i++) {
        comp[i].imag(floor(a[i] / d));
        comp[i].real(a[i] - comp[i].imag() * d);
    }
}

template <typename C = default_complex, typename T>
auto fft_split_multiply(const vector<T>& a, const vector<T>& b) {
    if (a.empty() || b.empty()) {
        return vector<T>();
    }
    int A = a.size(), B = b.size();
    int S = A + B - 1, s = next_two(S), N = 1 << s;
    if (1.0 * A * B <= 19.0 * N * s) {
        return naive_multiply(a, b);
    }

    T H = sqrt(max(maxabsolute(a), maxabsolute(b))), Q = H * H;
    auto [fa, fb] = fft_roots_cache<C>::get_scratch(N);
    fft_split_lower_upper(H, a, fa);
    fft_split_lower_upper(H, b, fb);
    fill_n(begin(fa) + A, N - A, C(0));
    fill_n(begin(fb) + B, N - B, C(0));
    fft_transform<0, 0>(fa, N);
    fft_transform<0, 0>(fb, N);
    vector<C> h0(N), h1(N);
    for (int i = 0, j = 0; i < N; i++, j = N - i) {
        auto f_small = (fa[i] + conj(fa[j])) * 0.5;
        auto f_large = (fa[i] - conj(fa[j])) * C(0, -0.5);
        auto g_small = (fb[i] + conj(fb[j])) * 0.5;
        auto g_large = (fb[i] - conj(fb[j])) * C(0, -0.5);
        h0[i] = f_small * g_small + C(0, 1) * f_large * g_large;
        h1[i] = f_small * g_large + f_large * g_small;
    }
    fft_transform<1, 0>(h0, N);
    fft_transform<1, 0>(h1, N);

    vector<T> c(S);
    for (int i = 0; i < S; i++) {
        T c0 = fft_round<T>(h0[i].real());
        T c1 = fft_round<T>(h1[i].real());
        T c2 = fft_round<T>(h0[i].imag());
        c[i] = c0 + c1 * H + c2 * Q;
    }
    trim_vector(c);
    return c;
}

} // namespace fft

// NTT-SPLIT for modnums (2 ffts) | 110,250,520ms for 250K,500K,1M
namespace fft {

template <uint32_t MOD, typename T = int, typename C>
auto fft_split_lower_upper_mod(T d, const vector<modnum<MOD>>& a, vector<C>& comp) {
    for (int i = 0, A = a.size(); i < A; i++) {
        T y = T(a[i]), v = abs(y) <= abs(y - T(MOD)) ? y : y - T(MOD);
        comp[i] = C(v % d, v / d);
    }
}

template <typename C = default_complex, uint32_t MOD>
auto fft_multiply(const vector<modnum<MOD>>& a, const vector<modnum<MOD>>& b) {
    using T = modnum<MOD>;
    if (a.empty() || b.empty()) {
        return vector<T>();
    }
    int A = a.size(), B = b.size();
    int S = A + B - 1, s = next_two(S), N = 1 << s;
    if (1.0 * A * B <= 3.5 * N * s) {
        return naive_multiply(a, b);
    }

    int H = sqrt(MOD) + 3, Q = H * H;
    auto [fa, fb] = fft_roots_cache<C>::get_scratch(N);
    fft_split_lower_upper_mod(H, a, fa);
    fft_split_lower_upper_mod(H, b, fb);
    fill_n(begin(fa) + A, N - A, C(0));
    fill_n(begin(fb) + B, N - B, C(0));
    fft_transform<0, 0>(fa, N);
    fft_transform<0, 0>(fb, N);
    vector<C> h0(N), h1(N);
    for (int i = 0, j = 0; i < N; i++, j = N - i) {
        auto f_small = (fa[i] + conj(fa[j])) * 0.5;
        auto f_large = (fa[i] - conj(fa[j])) * C(0, -0.5);
        auto g_small = (fb[i] + conj(fb[j])) * 0.5;
        auto g_large = (fb[i] - conj(fb[j])) * C(0, -0.5);
        h0[i] = f_small * g_small + C(0, 1) * f_large * g_large;
        h1[i] = f_small * g_large + f_large * g_small;
    }
    fft_transform<1, 0>(h0, N);
    fft_transform<1, 0>(h1, N);

    vector<T> c(S);
    for (int i = 0; i < S; i++) {
        T c0 = fft_round<int64_t>(h0[i].real()) % MOD;
        T c1 = fft_round<int64_t>(h1[i].real()) % MOD;
        T c2 = fft_round<int64_t>(h0[i].imag()) % MOD;
        c[i] = c0 + c1 * H + c2 * Q;
    }
    trim_vector(c);
    return c;
}

} // namespace fft

// NTT-SPLIT without modnums (2 ffts) | 110,250,520ms for 250K,500K,1M
namespace fft {

template <typename T, typename O>
T fitmod(T mod, O val) {
    return val < 0 ? (val + mod) : (val >= mod) ? (val - mod) : val;
}

template <typename Prom, typename T, typename C>
void fft_split_lower_upper_mod(T mod, T d, const vector<T>& a, vector<C>& comp) {
    for (int i = 0, A = a.size(); i < A; i++) {
        T v = abs(a[i]) <= abs(a[i] - mod) ? a[i] : a[i] - mod;
        comp[i] = C(v % d, v / d);
    }
}

template <typename Prom = int64_t, typename C = default_complex, typename T>
auto naive_multiply(T mod, const vector<T>& a, const vector<T>& b) {
    int A = a.size(), B = b.size(), S = A && B ? A + B - 1 : 0;
    vector<T> c(S);
    for (int i = 0; i < A && B; i++) {
        for (int j = 0; j < B; j++) {
            c[i + j] = (c[i + j] + Prom(a[i]) * Prom(b[j])) % mod;
        }
    }
    for (int i = 0; i < S; i++) {
        if (c[i] < 0) {
            c[i] += mod;
        }
    }
    trim_vector(c);
    return c;
}

template <typename Prom = int64_t, typename C = default_complex, typename T>
auto fft_multiply(T mod, const vector<T>& a, const vector<T>& b) {
    if (a.empty() || b.empty()) {
        return vector<T>();
    }
    int A = a.size(), B = b.size();
    int S = A + B - 1, s = next_two(S), N = 1 << s;
    if (1.0 * A * B <= 3.5 * N * s) {
        return naive_multiply<Prom>(mod, a, b);
    }

    T H = sqrt(mod) + 3, Q = H * H;
    auto [fa, fb] = fft_roots_cache<C>::get_scratch(N);
    fft_split_lower_upper_mod<Prom>(mod, H, a, fa);
    fft_split_lower_upper_mod<Prom>(mod, H, b, fb);
    fill_n(begin(fa) + A, N - A, C(0));
    fill_n(begin(fb) + B, N - B, C(0));
    fft_transform<0, 0>(fa, N);
    fft_transform<0, 0>(fb, N);
    vector<C> h0(N), h1(N);
    for (int i = 0, j = 0; i < N; i++, j = N - i) {
        auto f_small = (fa[i] + conj(fa[j])) * 0.5;
        auto f_large = (fa[i] - conj(fa[j])) * C(0, -0.5);
        auto g_small = (fb[i] + conj(fb[j])) * 0.5;
        auto g_large = (fb[i] - conj(fb[j])) * C(0, -0.5);
        h0[i] = f_small * g_small + C(0, 1) * f_large * g_large;
        h1[i] = f_small * g_large + f_large * g_small;
    }
    fft_transform<1, 0>(h0, N);
    fft_transform<1, 0>(h1, N);

    vector<T> c(S);
    for (int i = 0; i < S; i++) {
        Prom c0 = fitmod(mod, fft_round<Prom>(h0[i].real()) % mod);
        Prom c1 = fitmod(mod, fft_round<Prom>(h1[i].real()) % mod);
        Prom c2 = fitmod(mod, fft_round<Prom>(h0[i].imag()) % mod);
        c[i] = fitmod(mod, c0 + fitmod(mod, c1 * H % mod + fitmod(mod, c2 * Q % mod)));
    }
    trim_vector(c);
    return c;
}

} // namespace fft
