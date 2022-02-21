#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename D, typename Fn>
auto simpson(int N, D l, D r, Fn&& f) {
    N += N & 1;
    D ans = 0;
    for (int i = 0; i <= N; i++) {
        D x = ((N - i) * l + i * r) / N;
        D y = f(x);
        ans += (i == 0 || i == N) ? y : (i & 1) ? 4 * y : 2 * y;
    }
    D h = (r - l) / N;
    return ans * h / 3;
}

template <typename D, typename Fn>
auto newton(int N, D x, D dh, Fn&& f) {
    static constexpr D maxnewton = 1e5;
    while (N--) {
        D v0 = f(x), v1 = f(x + dh);
        D d = std::clamp(dh / (v1 - v0), -maxnewton, maxnewton);
        x -= v0 * d;
    }
    return x;
}

// Solve explicit 1st order ODE s.t.   a'=f(t,a)
// Initial condition (t0,a0) and up to t1, with N steps. RK4
template <typename D, typename Fn>
auto explicit_rk4_one(int N, D tstart, D astart, D tend, Fn&& f) {
    vector<array<D, 2>> trace(N + 1);
    D h = (tend - tstart) / N;
    trace[0] = {tstart, astart};

    for (int i = 0; i < N; i++) {
        auto [t, a] = trace[i];

        D a0 = h * f(t, a);
        D a1 = h * f(t + h / 2, a + a0 / 2);
        D a2 = h * f(t + h / 2, a + a1 / 2);
        D a3 = h * f(t + h, a + a2);

        D da = (a0 + 2 * a1 + 2 * a2 + a3) / 6;
        D tn = ((N - i - 1) * tstart + (i + 1) * tend) / N;
        trace[i + 1] = {tn, a + da};
    }
    return trace;
}

// Solve explicit 2nd order ODE s.t.   a'=f(t,a,b)    b'=g(t,a,b)
// Initial condition (t0,a0,b0) and up to t1, with N steps. RK4
template <typename D, typename Fn, typename Gn>
auto explicit_rk4_two(int N, D tstart, D astart, D bstart, D tend, Fn&& f, Gn&& g) {
    vector<array<D, 3>> trace(N + 1);
    D h = (tend - tstart) / N;
    trace[0] = {tstart, astart, bstart};

    for (int i = 0; i < N; i++) {
        auto [t, a, b] = trace[i];

        D a0 = h * f(t, a, b);
        D b0 = h * g(t, a, b);
        D a1 = h * f(t + h / 2, a + a0 / 2, b + b0 / 2);
        D b1 = h * g(t + h / 2, a + a0 / 2, b + b0 / 2);
        D a2 = h * f(t + h / 2, a + a1 / 2, b + b1 / 2);
        D b2 = h * g(t + h / 2, a + a1 / 2, b + b1 / 2);
        D a3 = h * f(t + h, a + a2, b + b2);
        D b3 = h * g(t + h, a + a2, b + b2);

        D da = (a0 + 2 * a1 + 2 * a2 + a3) / 6;
        D db = (b0 + 2 * b1 + 2 * b2 + b3) / 6;
        D tn = ((N - i - 1) * tstart + (i + 1) * tend) / N;
        trace[i + 1] = {tn, a + da, b + db};
    }
    return trace;
}

// Solve implicit 1st order ODE s.t. f(t,a,a')=0
// Use newton's with S steps to compute a', and you provide guess for first slope
// Initial condition (t0,a0) and up to t1, with N steps. RK1
template <typename D, typename Fn>
auto implicit_rk1_one(int N, int S, D tstart, D astart, D tend, D slope, D dh, Fn&& f) {
    static constexpr D maxnewton = 1e5; // maximum inverse of derivative in newton

    auto newton = [&](D t, D a, D x) {
        int runs = S;
        while (runs--) {
            auto v0 = f(t, a, x);
            auto v1 = f(t, a, x + dh);
            auto d = std::clamp(dh / (v1 - v0), -maxnewton, maxnewton);
            x -= v0 * d;
        }
        return x;
    };

    vector<array<D, 3>> trace(N + 1);
    D h = (tend - tstart) / N;
    trace[0] = {tstart, astart, newton(tstart, astart, slope)};

    for (int i = 0; i < N; i++) {
        auto [t, a, b] = trace[i];

        D da = h * b;
        D bn = newton(t + h, a + da, b);
        D tn = ((N - i - 1) * tstart + (i + 1) * tend) / N;
        trace[i + 1] = {tn, a + da, bn};
    }
    return trace;
}

// Solve implicit 2nd order separated ODE s.t. f(t,a,b,a')=0   g(t,a,b,b')=0
// Use newton's with S steps to compute a' and b', and you provide guess for first slopes
// Initial condition (t0,a0) and up to t1, with N steps. RK1
template <typename D, typename Fn, typename Gn>
auto implicit_rk1_two(int N, int S, D tstart, D astart, D bstart, D tend, D aslope,
                      D bslope, D dh, Fn&& f, Gn&& g) {
    static constexpr D maxnewton = 1e5; // maximum inverse of derivative in newton

    auto newton = [&](D t, D a, D b, D x, auto& fn) {
        int runs = S;
        while (runs--) {
            auto v0 = fn(t, a, b, x);
            auto v1 = fn(t, a, b, x + dh);
            auto d = std::clamp(dh / (v1 - v0), -maxnewton, maxnewton);
            x -= v0 * d;
        }
        return x;
    };

    vector<array<D, 5>> trace(N + 1);
    D first_a_slope = newton(tstart, astart, bstart, aslope, f);
    D first_b_slope = newton(tstart, astart, bstart, bslope, g);
    D h = (tend - tstart) / N;
    trace[0] = {tstart, astart, bstart, first_a_slope, first_b_slope};

    for (int i = 0; i < N; i++) {
        auto [t, a, b, c, d] = trace[i];

        D da = h * c;
        D db = h * d;
        D cn = newton(t + h, a + da, b + db, c, f);
        D dn = newton(t + h, a + da, b + db, d, g);
        D tn = ((N - i - 1) * tstart + (i + 1) * tend) / N;
        trace[i + 1] = {tn, a + da, b + db, cn, dn};
    }
    return trace;
}
