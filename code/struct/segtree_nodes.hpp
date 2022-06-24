#pragma once

#include "numeric/modnum.hpp"

struct sum_segnode {
    using V = int64_t;
    static constexpr bool LAZY = true, RANGES = true;
    V value = 0, lazy = 0;

    sum_segnode(V value = 0) : value(value) {}
    operator V() const { return value; }

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

    void apply(V add, int s) {
        value += s * add;
        lazy += add;
    }
};

struct polyhash_segnode {
    using V = modnum<998244353>;
    static constexpr bool LAZY = false, RANGES = false;

    static inline vector<V> polypow;
    static void init(int N, V b) {
        polypow.resize(N + 1);
        polypow[0] = 1;
        for (int i = 1; i <= N; i++) {
            polypow[i] = polypow[i - 1] * b;
        }
    }

    int size = 0;
    V value = 0;

    polyhash_segnode() = default;
    polyhash_segnode(V v) : size(1), value(v) {}
    operator V const&() const { return value; }

    void pushup(const polyhash_segnode& lhs, const polyhash_segnode& rhs) {
        size = lhs.size + rhs.size;
        value = lhs.value + rhs.value * polypow[lhs.size];
    }
    void apply(V v) { value = v; }
};

struct affine_segnode {
    using V = modnum<998244353>;
    static constexpr bool LAZY = false, RANGES = false;

    using Data = array<V, 2>; // f(x) = [0]x + [1]
    static constexpr Data identity = {1, 0};
    static Data combine(Data a, Data b) { return Data{a[0] * b[0], a[0] * b[1] + a[1]}; }

    array<Data, 2> f = {identity, identity}; // [0] = lmr, [1] = rml

    affine_segnode() = default;
    affine_segnode(V a, V b) : affine_segnode(Data{a, b}) {}
    affine_segnode(Data g) : f({g, g}) {}

    void reverse() { swap(f[0], f[1]); }
    V eval_lmr(V x) const { return f[0][0] * x + f[0][1]; }
    V eval_rml(V x) const { return f[1][0] * x + f[1][1]; }

    void pushup(const affine_segnode& lhs, const affine_segnode& rhs) {
        f[0] = combine(lhs.f[0], rhs.f[0]);
        f[1] = combine(rhs.f[1], lhs.f[1]);
    }
    void apply(Data fn) { f[0] = f[1] = fn; }
};

struct maxsubrange_segnode {
    using V = int64_t;
    static constexpr bool LAZY = false, RANGES = false;
    static constexpr V MIN = numeric_limits<V>::lowest() / 2;

    V sum = 0, pref = MIN, suff = MIN, best = MIN;

    maxsubrange_segnode() = default;
    maxsubrange_segnode(V v) : sum(v), pref(v), suff(v), best(v) {}
    operator V() const { return best; }

    void pushup(const maxsubrange_segnode& lhs, const maxsubrange_segnode& rhs) {
        assert(lhs.sum != MIN && rhs.sum != MIN);
        sum = lhs.sum + rhs.sum;
        pref = max(lhs.pref, lhs.sum + rhs.pref);
        suff = max(rhs.suff, lhs.suff + rhs.sum);
        best = max({lhs.best, rhs.best, lhs.suff + rhs.pref});
    }

    void apply(V add) {
        sum += add;
        pref = suff = best = sum;
    }
};

struct gcd_segnode {
    using V = int64_t;
    static constexpr bool LAZY = true, RANGES = false;

    V value = 0, g = 0, diff = 0, lazy = 0;

    gcd_segnode() = default;
    gcd_segnode(V v) : value(v) {}
    operator V() const { return g; }

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
    void apply(V add) {
        value += add;
        g = gcd(value, diff);
        lazy += add;
    }
};

struct matmul_segnode {
    using V = modnum<998244353>;
    static constexpr bool LAZY = false, RANGES = false;
    static constexpr int M = 3;
    using Mat = array<array<V, M>, M>;
    Mat A = {};

    matmul_segnode() {
        for (int i = 0; i < M; i++)
            A[i][i] = 1;
    }
    matmul_segnode(Mat A) : A(A) {}

    void pushup(const matmul_segnode& lhs, const matmul_segnode& rhs) {
        A = {};
        for (int i = 0; i < M; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < M; k++)
                    A[i][j] += lhs.A[i][k] * rhs.A[k][j];
    }

    void apply(Mat B) { A = B; }
};

struct topmul_segnode {
    using V = int64_t;
    static constexpr bool LAZY = false, RANGES = false;
    static constexpr V inf = numeric_limits<V>::lowest() / 2;
    static constexpr int M = 3;
    using Mat = array<array<V, M>, M>;
    Mat A = {};

    static constexpr V add(V a, V b) { return max(a, b); }
    static constexpr V mul(V a, V b) { return a + b; }
    static void init(Mat& A) {
        for (int i = 0; i < M; i++)
            for (int j = 0; j < M; j++)
                A[i][j] = inf;
    }

    topmul_segnode() { init(A); }
    topmul_segnode(Mat A) : A(A) {}

    void pushup(const topmul_segnode& lhs, const topmul_segnode& rhs) {
        init(A);
        for (int i = 0; i < M; i++)
            for (int j = 0; j < M; j++)
                for (int k = 0; k < M; k++)
                    A[i][j] = add(A[i][j], mul(lhs.A[i][k], rhs.A[k][j]));
    }

    void apply(Mat B) { A = B; }
};

struct mincount_segnode {
    using V = int64_t;
    static constexpr bool LAZY = true, RANGES = false;
    static constexpr V MAX = numeric_limits<V>::max();

    V value = MAX, count = 0, lazy = 0;

    explicit mincount_segnode(V v = MAX) : value(v), count(v < MAX) {}

    void pushup(const mincount_segnode& lhs, const mincount_segnode& rhs) {
        value = min(lhs.value, rhs.value);
        count = (lhs.value <= value ? lhs.count : 0);
        count += (rhs.value <= value ? rhs.count : 0);
    }
    void pushdown(mincount_segnode& lhs, mincount_segnode& rhs) {
        if (lazy != 0) {
            lhs.apply(lazy);
            rhs.apply(lazy);
            lazy = 0;
        }
    }
    void apply(V add) {
        value += add;
        lazy += add;
    }
};

struct maxcount_segnode {
    using V = int64_t;
    static constexpr bool LAZY = true, RANGES = false;
    static constexpr V MIN = numeric_limits<int>::min();

    V value = MIN, count = 0, lazy = 0;

    explicit maxcount_segnode(V v = MIN) : value(v), count(v > MIN) {}

    void pushup(const maxcount_segnode& lhs, const maxcount_segnode& rhs) {
        value = max(lhs.value, rhs.value);
        count = (lhs.value >= value ? lhs.count : 0);
        count += (rhs.value >= value ? rhs.count : 0);
    }
    void pushdown(maxcount_segnode& lhs, maxcount_segnode& rhs) {
        if (lazy != 0) {
            lhs.apply(lazy);
            rhs.apply(lazy);
            lazy = 0;
        }
    }
    void apply(V add) {
        value += add;
        lazy += add;
    }
};

struct rangeset_segnode {
    using V = int;
    static constexpr bool LAZY = false, RANGES = false;
    static constexpr V MIN = numeric_limits<V>::lowest();
    static constexpr V MAX = numeric_limits<V>::max();
    V minimum = MAX, maximum = MIN;
    multiset<V> nums;

    rangeset_segnode() = default;
    rangeset_segnode(V v) { nums.insert(v); }

    // Use these to get the actual min/max on the range/point
    V getmin() const { return min(minimum, nums.empty() ? MAX : *nums.begin()); }
    V getmax() const { return max(maximum, nums.empty() ? MIN : *nums.rbegin()); }

    void pushup(const rangeset_segnode& a, const rangeset_segnode& b) {
        minimum = min(a.getmin(), b.getmin());
        maximum = max(a.getmax(), b.getmax());
    }

    void apply(bool add, V v) {
        if (add) {
            nums.insert(v);
        } else if (auto it = nums.find(v); it != nums.end()) {
            nums.erase(it);
        }
    }

    static auto visitor(rangeset_segnode& ans) {
        return [&](const auto& node, int, int) {
            rangeset_segnode res;
            res.pushup(ans, node);
            ans = res;
        };
    }
};

struct setmin_sum_segnode {
    using V = int64_t;
    static constexpr bool LAZY = true, RANGES = true;
    static constexpr V MIN = numeric_limits<V>::lowest();
    static constexpr V MAX = numeric_limits<V>::max();
    enum Operation { SETMIN, ADD };

    V sum = 0;
    V maximum = MIN;
    V second_max = MIN;
    V count_max = 0;
    V add_lazy = 0;

    setmin_sum_segnode() = default;
    setmin_sum_segnode(V v) : sum(v), maximum(v), count_max(1) {}

    bool can_break(Operation op, V setmin, int) const {
        return op != SETMIN || maximum <= setmin;
    }
    bool can_update(Operation op, V setmin, int) const {
        return op != SETMIN || second_max < setmin;
    }

    void pushup(const setmin_sum_segnode& lhs, const setmin_sum_segnode& rhs) {
        sum = lhs.sum + rhs.sum;
        maximum = max(lhs.maximum, rhs.maximum);
        if (maximum == lhs.maximum && maximum == rhs.maximum) {
            second_max = max(lhs.second_max, rhs.second_max);
            count_max = lhs.count_max + rhs.count_max;
        } else if (maximum == lhs.maximum) {
            second_max = max(lhs.second_max, rhs.maximum);
            count_max = lhs.count_max;
        } else {
            second_max = max(lhs.maximum, rhs.second_max);
            count_max = rhs.count_max;
        }
    }
    void pushdown(setmin_sum_segnode& lhs, setmin_sum_segnode& rhs, int a, int b) {
        if (add_lazy) {
            lhs.apply_add(add_lazy, a);
            rhs.apply_add(add_lazy, b);
            add_lazy = 0;
        }
        lhs.apply_setmin(maximum);
        rhs.apply_setmin(maximum);
    }
    void apply_setmin(V setmin) {
        if (setmin < maximum) {
            assert(second_max < setmin);
            sum -= count_max * (maximum - setmin);
            maximum = setmin;
        }
    }
    void apply_add(V v, int s) {
        sum += v * s;
        maximum += v;
        if (second_max != MIN)
            second_max += v;
        add_lazy += v;
    }
    void apply(Operation op, V v, int s) {
        if (op == SETMIN) {
            apply_setmin(v);
        } else if (op == ADD) {
            apply_add(v, s);
        }
    }
};

struct setmod_sum_segnode {
    using V = int64_t;
    static constexpr bool LAZY = false, RANGES = false;
    static constexpr V MIN = numeric_limits<V>::lowest();
    static constexpr V MAX = numeric_limits<V>::max();

    V sum = 0;
    V maximum = MIN;

    setmod_sum_segnode() = default;
    setmod_sum_segnode(V v) : sum(v), maximum(v) {}

    bool can_break(V setmod) const { return maximum < setmod; }
    bool can_update(V) const { return false; }

    void pushup(const setmod_sum_segnode& lhs, const setmod_sum_segnode& rhs) {
        sum = lhs.sum + rhs.sum;
        maximum = max(lhs.maximum, rhs.maximum);
    }
    void apply(V setmod) {
        if (setmod <= maximum) {
            sum = maximum = sum % setmod;
        }
    }
};

struct fullji_segnode {
    using V = int64_t;
    static constexpr bool LAZY = true, RANGES = true;
    static constexpr V MIN = numeric_limits<V>::lowest();
    static constexpr V MAX = numeric_limits<V>::max();
    enum Operation { SETMIN, SETMAX, ADD, SET };

    V sum = 0;
    V minimum = MAX;
    V maximum = MIN;
    V second_min = MAX;
    V second_max = MIN;
    int count_min = 0;
    int count_max = 0;

    bool has_set_lazy = false;
    V set_lazy = 0;
    V add_lazy = 0;

    fullji_segnode() = default;
    fullji_segnode(V v) : sum(v), minimum(v), maximum(v), count_min(1), count_max(1) {}

    bool can_break(Operation op, V v, int) const {
        if (op == SETMIN) {
            return v >= maximum;
        } else if (op == SETMAX) {
            return v <= minimum;
        } else {
            return false;
        }
    }
    bool can_update(Operation op, V v, int) const {
        if (op == SETMIN) {
            return v > second_max;
        } else if (op == SETMAX) {
            return v < second_min;
        } else {
            return true;
        }
    }

    void pushup(const fullji_segnode& lhs, const fullji_segnode& rhs) {
        sum = lhs.sum + rhs.sum;

        minimum = min(lhs.minimum, rhs.minimum);
        if (minimum == lhs.minimum && minimum == rhs.minimum) {
            second_min = min(lhs.second_min, rhs.second_min);
            count_min = lhs.count_min + rhs.count_min;
        } else if (minimum == lhs.minimum) {
            second_min = min(lhs.second_min, rhs.minimum);
            count_min = lhs.count_min;
        } else {
            second_min = min(lhs.minimum, rhs.second_min);
            count_min = rhs.count_min;
        }

        maximum = max(lhs.maximum, rhs.maximum);
        if (maximum == lhs.maximum && maximum == rhs.maximum) {
            second_max = max(lhs.second_max, rhs.second_max);
            count_max = lhs.count_max + rhs.count_max;
        } else if (maximum == lhs.maximum) {
            second_max = max(lhs.second_max, rhs.maximum);
            count_max = lhs.count_max;
        } else {
            second_max = max(lhs.maximum, rhs.second_max);
            count_max = rhs.count_max;
        }
    }
    void pushdown(fullji_segnode& lhs, fullji_segnode& rhs, int l, int r) {
        if (has_set_lazy) {
            lhs.apply_set(set_lazy, l);
            rhs.apply_set(set_lazy, r);
            add_lazy = set_lazy = has_set_lazy = 0;
        } else {
            if (add_lazy != 0) {
                lhs.apply_add(add_lazy, l);
                rhs.apply_add(add_lazy, r);
                add_lazy = 0;
            }
            lhs.apply_min(maximum, l);
            rhs.apply_min(maximum, r);
            lhs.apply_max(minimum, l);
            rhs.apply_max(minimum, r);
        }
    }
    void apply_set(V v, int s) {
        sum = v * s;
        minimum = v;
        maximum = v;
        second_min = MAX;
        second_max = MIN;
        count_min = s;
        count_max = s;
        has_set_lazy = true;
        set_lazy = v;
        add_lazy = 0;
    }
    void apply_min(V v, int s) {
        if (minimum >= v) {
            apply_set(v, s);
        } else if (maximum > v) {
            assert(second_max < v);
            if (second_min == maximum) {
                second_min = v;
            }
            sum -= (maximum - v) * count_max;
            maximum = v;
        }
    }
    void apply_max(V v, int s) {
        if (maximum <= v) {
            apply_set(v, s);
        } else if (minimum < v) {
            assert(second_min > v);
            if (second_max == minimum) {
                second_max = v;
            }
            sum += (v - minimum) * count_min;
            minimum = v;
        }
    }
    void apply_add(V v, int s) {
        if (minimum == maximum) {
            apply_set(minimum + v, s);
        } else {
            sum += v * s;
            minimum += v;
            maximum += v;
            second_min = second_min == MAX ? MAX : v + second_min;
            second_max = second_max == MIN ? MIN : v + second_max;
            add_lazy += v;
        }
    }
    void apply(Operation op, V v, int s) {
        if (op == SETMIN) {
            apply_min(v, s);
        } else if (op == SETMAX) {
            apply_max(v, s);
        } else if (op == ADD) {
            apply_add(v, s);
        } else if (op == SET) {
            apply_set(v, s);
        } else {
            assert(false && "Invalid operation");
        }
    }
};
