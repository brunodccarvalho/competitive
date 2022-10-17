#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T>
struct affine_segnode {
    static constexpr bool LAZY = false;
    using ipos = int;

    using Data = array<T, 2>; // f(x) = [0]x + [1]
    static constexpr Data identity = {1, 0};
    static Data combine(Data a, Data b) { return Data{a[0] * b[0], a[0] * b[1] + a[1]}; }
    static Data expcombine(Data a, ipos e) {
        Data f = identity;
        while (e > 0) {
            if (e & 1)
                f = combine(f, a);
            if (e >>= 1)
                a = combine(a, a);
        }
        return f;
    }

    Data self = identity;
    array<Data, 2> subt = {identity, identity}; // lmr, rml

    affine_segnode() = default;
    affine_segnode(T a, T b) : affine_segnode(Data{a, b}) {}
    affine_segnode(Data f) : self(f), subt({f, f}) {}

    T eval_lmr(T x) const { return subt[0][0] * x + subt[0][1]; }
    T eval_rml(T x) const { return subt[1][0] * x + subt[1][1]; }

    auto fn_point() const { return self; }
    auto fn_lmr() const { return subt[0]; }
    auto fn_rml() const { return subt[1]; }

    void update_self(Data f) { self = f; }

    static auto mult(ipos S, const affine_segnode& node) {
        affine_segnode ans;
        ans.self = node.self;
        ans.subt[0] = ans.subt[1] = expcombine(node.self, S);
        return ans;
    }

    void pushup(const affine_segnode& lhs, const affine_segnode& rhs, ipos S) {
        subt[0] = combine(combine(lhs.subt[0], expcombine(self, S)), rhs.subt[0]);
        subt[1] = combine(combine(rhs.subt[1], expcombine(self, S)), lhs.subt[1]);
    }
};

template <typename T>
struct sum_segnode {
    static constexpr bool LAZY = true;
    using ipos = int;

    T self = 0;
    T subt = 0;
    T lazy = 0;

    sum_segnode(T v = 0) : self(v) {}

    auto point_val() const { return self; }
    auto range_sum() const { return subt; }

    static auto mult(ipos unit_size, const sum_segnode& node) {
        sum_segnode ans;
        ans.self = node.self;
        ans.subt = node.self * unit_size;
        return ans;
    }

    void update_self(T update) { self += update; }

    void update_subt(T update, ipos subt_size) {
        if (subt_size) { // this guard is required
            self += update;
            subt += update * subt_size;
            lazy += update;
        }
    }

    void pushup(const sum_segnode& lhs, const sum_segnode& rhs, ipos self_size) {
        subt = self * self_size + lhs.subt + rhs.subt;
    }

    void pushdown(sum_segnode& lhs, sum_segnode& rhs, ipos L, ipos R) {
        if (lazy != 0) {
            lhs.update_subt(lazy, L);
            rhs.update_subt(lazy, R);
            lazy = 0;
        }
    }
};

template <typename T>
struct sum_affine_segnode {
    static constexpr bool LAZY = true;
    using ipos = int;

    T self = 0;
    T subt = 0;
    T b = 1, c = 0; // lazy tags

    sum_affine_segnode(T v = 0) : self(v) {}

    auto point_val() const { return self; }
    auto range_sum() const { return subt; }

    static auto mult(ipos S, const sum_affine_segnode& node) {
        sum_affine_segnode ans;
        ans.self = node.self;
        ans.subt = node.self * S;
        return ans;
    }

    void update_self(pair<T, T> add) { self = add.first * self + add.second; }

    void update_subt(pair<T, T> add, ipos subt_size) {
        if (subt_size) {
            auto [x, y] = add;
            self = x * self + y;
            subt = x * subt + y * subt_size;
            b = b * x, c = c * x + y;
        }
    }

    void pushup(const sum_affine_segnode& lhs, const sum_affine_segnode& rhs, ipos S) {
        subt = self * S + lhs.subt + rhs.subt;
    }

    void pushdown(sum_affine_segnode& lhs, sum_affine_segnode& rhs, ipos L, ipos R) {
        if (b != 1 || c != 0) {
            lhs.update_subt({b, c}, L);
            rhs.update_subt({b, c}, R);
            b = 1, c = 0;
        }
    }
};

template <typename T>
struct min_segnode {
    static constexpr bool LAZY = true;
    using ipos = int;

    static constexpr T MAX = numeric_limits<T>::max();
    T self = MAX;
    T subt = MAX;
    T lazy = 0;

    min_segnode() = default;
    min_segnode(T v) : self(v) {}

    auto point_val() const { return self; }
    auto range_min() const { return subt; }

    static auto mult(ipos, const min_segnode& node) {
        min_segnode ans;
        ans.self = node.self;
        ans.subt = node.self;
        return ans;
    }

    void update_self(T add) { self += add; }
    void update_subt(T add, ipos S) {
        if (S) {
            self += add;
            subt += add;
            lazy += add;
        }
    }

    void pushup(const min_segnode& lhs, const min_segnode& rhs, ipos) {
        subt = min({self, lhs.subt, rhs.subt});
    }

    void pushdown(min_segnode& lhs, min_segnode& rhs, ipos L, ipos R) {
        if (lazy != 0) {
            lhs.update_subt(lazy, L);
            rhs.update_subt(lazy, R);
            lazy = 0;
        }
    }
};

template <typename T>
struct max_segnode {
    static constexpr bool LAZY = true;
    using ipos = int;

    static constexpr T MIN = numeric_limits<T>::lowest();
    T self = MIN;
    T subt = MIN;
    T lazy = 0;

    max_segnode() = default;
    max_segnode(T v) : self(v) {}

    auto point_val() const { return self; }
    auto range_max() const { return subt; }

    static auto mult(ipos, const max_segnode& node) {
        max_segnode ans;
        ans.self = node.self;
        ans.subt = node.self;
        return ans;
    }

    void update_self(T add) { self += add; }

    void update_subt(T add, ipos S) {
        if (S) {
            self += add;
            subt += add;
            lazy += add;
        }
    }

    void pushup(const max_segnode& lhs, const max_segnode& rhs, ipos) {
        subt = max({self, lhs.subt, rhs.subt});
    }

    void pushdown(max_segnode& lhs, max_segnode& rhs, ipos L, ipos R) {
        if (lazy != 0) {
            lhs.update_subt(lazy, L);
            rhs.update_subt(lazy, R);
            lazy = 0;
        }
    }
};

template <typename T>
struct gcd_segnode {
    static constexpr bool LAZY = true;
    using ipos = int;

    T self = 0;
    T subt = 0;
    T diff = 0;
    T lazy = 0;

    auto point_val() const { return self; }
    auto range_gcd() const { return subt < 0 ? -subt : subt; }

    static auto mult(ipos S, const gcd_segnode& node) {
        assert(S > 0);
        gcd_segnode ans;
        ans.self = node.self;
        ans.subt = node.self; // required
        ans.diff = 0;
        return ans;
    }

    void update_self(T add) { self += add; }

    void update_subt(T add, ipos S) {
        if (S) {
            self += add;
            subt = gcd(self, diff);
            lazy += add;
        }
    }

    void pushup(const gcd_segnode& lhs, const gcd_segnode& rhs, ipos) {
        diff = gcd(gcd(lhs.diff, rhs.diff), gcd(self - lhs.self, self - rhs.self));
        subt = gcd(self, diff);
    }

    void pushdown(gcd_segnode& lhs, gcd_segnode& rhs, ipos L, ipos R) {
        if (lazy != 0) {
            lhs.update_subt(lazy, L);
            rhs.update_subt(lazy, R);
            lazy = 0;
        }
    }
};
