#pragma once

#include "numeric/modnum.hpp"

struct sum_segnode {
    static constexpr bool LAZY = true, RANGES = true, REVERSE = false;
    long value = 0, lazy = 0;

    sum_segnode(long value = 0) : value(value) {}
    operator long() const { return value; }

    void pushup(const sum_segnode& lhs, const sum_segnode& rhs) {
        assert(lazy == 0);
        value = lhs.value + rhs.value;
    }

    void pushdown(sum_segnode& lhs, sum_segnode& rhs, int a, int b) {
        if (lazy) {
            lhs.apply(lazy, a);
            rhs.apply(lazy, b);
            lazy = 0;
        }
    }

    void apply(long add, int s) {
        value += s * add;
        lazy += add;
    }
};

struct polyhash_segnode {
    using T = modnum<998244353>;
    static constexpr bool LAZY = false, RANGES = false, REVERSE = false;

    static inline vector<T> polypow;
    static void init(int N, T b) {
        polypow.resize(N + 1);
        polypow[0] = 1;
        for (int i = 1; i <= N; i++) {
            polypow[i] = polypow[i - 1] * b;
        }
    }

    int size = 0;
    T value = 0;

    polyhash_segnode() = default;
    polyhash_segnode(T v) : size(1), value(v) {}
    operator T const &() const { return value; }

    void pushup(const polyhash_segnode& lhs, const polyhash_segnode& rhs) {
        size = lhs.size + rhs.size;
        value = lhs.value + rhs.value * polypow[lhs.size];
    }
    void apply(T v) { value = v; }
};

struct affine_segnode {
    using T = modnum<998244353>;
    static constexpr bool LAZY = false, RANGES = false, REVERSE = true;

    using Data = array<T, 2>; // f(x) = [0]x + [1]
    static constexpr Data identity = {1, 0};
    static Data combine(Data a, Data b) { return Data{a[0] * b[0], a[0] * b[1] + a[1]}; }

    Data f[2] = {identity, identity}; // [0] = lmr, [1] = rml

    affine_segnode() = default;
    affine_segnode(T a, T b) : affine_segnode(Data{a, b}) {}
    affine_segnode(Data f) : f({f, f}) {}

    void reverse() { swap(f[0], f[1]); }
    T eval_lmr(T x) const { return f[0][0] * x + f[0][1]; }
    T eval_rml(T x) const { return f[1][0] * x + f[1][1]; }

    void pushup(const affine_segnode& lhs, const affine_segnode& rhs) {
        f[0] = combine(lhs.f[0], rhs.f[0]);
        f[1] = combine(rhs.f[1], lhs.f[1]);
    }
    void apply(Data fn) { f[0] = f[1] = fn; }
};

struct maxsubrange_segnode {
    static constexpr bool LAZY = false, RANGES = false, REVERSE = false;
    static constexpr int MIN = numeric_limits<int>::lowest() / 2;

    int sum = 0, pref = MIN, suff = MIN, best = MIN;

    maxsubrange_segnode() = default;
    maxsubrange_segnode(int v) : sum(v), pref(v), suff(v), best(v) {}
    operator int const &() const { return best; }

    void pushup(const maxsubrange_segnode& lhs, const maxsubrange_segnode& rhs) {
        assert(lhs.sum != MIN && rhs.sum != MIN);
        sum = lhs.sum + rhs.sum;
        pref = max(lhs.pref, lhs.sum + rhs.pref);
        suff = max(rhs.suff, lhs.suff + rhs.sum);
        best = max({lhs.best, rhs.best, lhs.suff + rhs.pref});
    }
    void apply(int add) {
        sum += add;
        pref = suff = best = sum;
    }
};

struct gcd_segnode {
    static constexpr bool LAZY = true, RANGES = false, REVERSE = false;

    int value = 0, g = 0, diff = 0;
    int lazy = 0;

    gcd_segnode() = default;
    gcd_segnode(int v) : value(v) {}
    operator int const &() const { return g; }

    void pushup(const gcd_segnode& lhs, const gcd_segnode& rhs) {
        value = lhs.value;
        diff = gcd(gcd(lhs.diff, rhs.diff), lhs.value - rhs.value);
        g = gcd(value, diff);
    }
    void pushdown(gcd_segnode& lhs, gcd_segnode& rhs) {
        if (lazy != 0) {
            lhs.apply(lazy);
            rhs.apply(lazy);
            lazy = 0;
        }
    }
    void apply(int add) {
        value += add;
        g = gcd(value, diff);
        lazy += add;
    }
};

struct mincount_segnode {
    static constexpr bool LAZY = true, RANGES = false, REVERSE = false;
    static constexpr int MAX = numeric_limits<int>::max();

    int value = MAX;
    int count = 1;
    int lazy = 0;

    mincount_segnode() = default;
    mincount_segnode(int v) : value(v) {}

    void pushup(const mincount_segnode& lhs, const mincount_segnode& rhs) {
        if (lhs.value < rhs.value) {
            *this = lhs;
        } else if (rhs.value < lhs.value) {
            *this = rhs;
        } else {
            value = rhs.value, count = lhs.count + rhs.count;
        }
    }
    void pushdown(mincount_segnode& lhs, mincount_segnode& rhs) {
        if (lazy != 0) {
            lhs.apply(lazy);
            rhs.apply(lazy);
            lazy = 0;
        }
    }
    void apply(int add) {
        value += add;
        lazy += add;
    }
};

struct maxcount_segnode {
    static constexpr bool LAZY = true, RANGES = false, REVERSE = false;
    static constexpr int MIN = numeric_limits<int>::lowest();

    int value = MIN;
    int count = 0;
    int lazy = 0;

    maxcount_segnode() = default;
    maxcount_segnode(int v) : value(v), count(1) {}

    void pushup(const maxcount_segnode& lhs, const maxcount_segnode& rhs) {
        if (lhs.value < rhs.value) {
            value = rhs.value, count = rhs.count;
        } else if (rhs.value < lhs.value) {
            value = lhs.value, count = lhs.count;
        } else {
            value = lhs.value, count = lhs.count + rhs.count;
        }
    }
    void pushdown(maxcount_segnode& lhs, maxcount_segnode& rhs) {
        if (lazy != 0) {
            lhs.apply(lazy);
            rhs.apply(lazy);
            lazy = 0;
        }
    }
    void apply(int add) {
        value += add;
        lazy += add;
    }
};
