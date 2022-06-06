#pragma once

#include <bits/stdc++.h>
using namespace std;

// Piecewise-linear unbounded convex function with slope changing points stored in heaps
// Supports pointwise addition of functions but not min-plus convolution
// Source: https://github.com/saketh-are/algo-lib
template <typename V>
struct LinPoints {
    static constexpr V ninf = numeric_limits<V>::lowest() / 2;
    static constexpr V pinf = numeric_limits<V>::max() / 2;
    using max_heap = priority_queue<pair<V, V>, vector<pair<V, V>>, less<pair<V, V>>>;
    using min_heap = priority_queue<pair<V, V>, vector<pair<V, V>>, greater<pair<V, V>>>;

    max_heap L;
    min_heap R;
    V y, Loff = 0, Roff = 0;
    // valley is top(L)..top(R), value there is y, Loff/Roff are lazily added to points

    // f(x) = 0
    LinPoints() = default;

    int merge_size() const { return L.size() + R.size(); }

    static LinPoints constant(V y) {
        LinPoints fn;
        fn.y = y;
        return fn;
    }

    // f(x) = {y if x<=M | a(x-M)+y if M<=x}
    static LinPoints valley_right(V a, V M, V y) {
        assert(a > 0);
        LinPoints fn;
        fn.y = y;
        fn.push_right(M, a);
        return fn;
    }

    // f(x) = {a(M-x)+y if x<=M | y if M<=x}
    static LinPoints valley_left(V a, V M, V y) {
        assert(a > 0);
        LinPoints fn;
        fn.y = y;
        fn.push_left(M, a);
        return fn;
    }

    // f(x) = {a(L-x)+y if x<=L | y if L<=x<=R | b(x-R)+y if R<=x}
    static LinPoints valley(V a, V b, V L, V R, V y) {
        assert(L <= R && a > 0 && b > 0);
        LinPoints fn;
        fn.y = y;
        fn.push_left(L, a);
        fn.push_right(R, b);
        return fn;
    }

    // f(x) = {a(p-x)+y if x<=p | b(x-p)+y if p<=x}
    static LinPoints abs(V a, V b, V p, V y) { return valley(a, b, p, p, y); }

    static void pointwise(LinPoints& fn, LinPoints& gn) {
        if (fn.merge_size() < gn.merge_size()) {
            swap(fn, gn);
        }
        fn.y += gn.y;
        while (gn.L.size()) {
            auto [x, d] = pop_heap(gn.L, gn.Loff);
            if (x <= fn.right_argmin()) {
                fn.push_left(x, d);
                continue;
            }
            auto p = fn.right_argmin();
            fn.y += (x - p) * d;
            fn.push_right(x, d);
            while (d > 0) {
                auto [r, dr] = pop_heap(fn.R, fn.Roff);
                if (dr >= d) {
                    fn.push_left(p, d);
                    fn.push_right(p, dr - d);
                    break;
                } else {
                    d -= dr;
                    fn.y -= d * (fn.right_argmin() - p);
                    p = fn.right_argmin();
                }
            }
        }
        while (gn.R.size()) {
            auto [x, d] = pop_heap(gn.R, gn.Roff);
            if (x >= fn.left_argmin()) {
                fn.push_right(x, d);
                continue;
            }
            auto p = fn.left_argmin();
            fn.y += (p - x) * d;
            fn.push_left(x, d);
            while (d > 0) {
                auto [l, dl] = pop_heap(fn.L, fn.Loff);
                if (dl >= d) {
                    fn.push_right(p, d);
                    fn.push_left(p, dl - d);
                    break;
                } else {
                    d -= dl;
                    fn.y -= d * (p - fn.left_argmin());
                    p = fn.left_argmin();
                }
            }
        }
    }

    // f(x) := g(x+c)
    auto shift(int c) { Loff -= c, Roff -= c; }

    // f(x) := g(x)+dy
    void add_constant(V dy) { y += dy; }

    // f(x) := min{da+x<=y<=db+y} g(y)
    void range_min(V da, V db) { assert(da <= db), Loff -= db, Roff -= da; }

    // f(x) := min{x<=y} g(y). Clears the left heap.
    void suffix_min() { L.swap(min_heap()), Loff = 0; }

    // f(x) := min{y<=x} g(y). Clears the right heap.
    void prefix_min() { R.swap(min_heap()), Roff = 0; }

    V minimum() const { return y; }
    V left_argmin() const { return L.empty() ? ninf : (L.top().first + Loff); }
    V right_argmin() const { return R.empty() ? pinf : (R.top().first + Roff); }
    auto valley() const { return make_pair(left_argmin(), right_argmin()); }

    auto destructive_query(V x) {
        V a = left_argmin(), b = right_argmin();
        if (a <= x && x <= b) {
            return y;
        } else if (x < a) {
            V slope = 0, ans = y;
            while (L.size()) {
                auto [f, d] = pop_heap(L, Loff);
                auto advance = min(a - x, a - f);
                ans += slope * advance;
                a -= advance, slope += d;
            }
            ans += slope * (a - x);
            return ans;
        } else /* b < x */ {
            V slope = 0, ans = y;
            while (R.size()) {
                auto [f, d] = pop_heap(R, Roff);
                auto advance = min(x - b, f - b);
                ans += slope * advance;
                b += advance, slope += d;
            }
            ans += slope * (x - b);
            return ans;
        }
    }

  private:
    template <typename Heap>
    static auto pop_heap(Heap& heap, V off) {
        auto [x, slope] = heap.top();
        V ans = 0;
        do {
            ans += heap.top().second, heap.pop();
        } while (heap.size() && heap.top().first == x);
        return make_pair(x + off, ans);
    }
    void push_left(V x, V slope) {
        if (slope) {
            L.emplace(x - Loff, slope);
        }
    }
    void push_right(V x, V slope) {
        if (slope) {
            R.emplace(x - Roff, slope);
        }
    }
};

// Piecewise-linear bounded convex function with slope values stored in heaps
// Supports min-plus convolution but not pointwise addition
template <typename V>
struct LinSlopes {
    using min_heap = priority_queue<pair<V, V>, vector<pair<V, V>>, greater<pair<V, V>>>;
    static constexpr V inf = numeric_limits<V>::max() / 2;
    min_heap L, R;
    V a = 0, b = 0, y = 0, Loff = 0, Roff = 0;
    // valley is [a,b], value there is y, Loff/Roff are lazily added to slopes

    // f(x) = {0 for x=0}
    LinSlopes() = default;

    static LinSlopes point(V x, V y) {
        LinSlopes fn;
        fn.a = fn.b = x, fn.y = y;
        return fn;
    }

    // f(x) = {y for L<=x<=R}
    static LinSlopes constant(V L, V R, V y) {
        assert(L <= R);
        LinSlopes fn;
        fn.a = L, fn.a = R, fn.y = y;
        return fn;
    }

    // f(x) = {c(x-L)+y for L<=x<=R}
    static LinSlopes slope(V c, V L, V R, V y) {
        assert(L <= R);
        LinSlopes fn;
        if (c < 0) {
            fn.a = fn.a = R, fn.y = y;
            fn.push_left(-c, R - L);
        } else if (c > 0) {
            fn.a = fn.a = L, fn.y = y;
            fn.push_right(c, R - L);
        } else {
            fn.a = L, fn.a = R, fn.y = y;
        }
    }

    // f(x) = {a(L-x)+y if L<=x<=A, y if A<=x<=B, b(R-x)+y if B<=x<=R | for L<=x<=R}
    static LinSlopes valley(V a, V b, V L, V A, V B, V R, V y) {
        assert(L <= A && A <= B && B <= R && a > 0 && b > 0);
        LinSlopes fn;
        fn.a = A, fn.b = B, fn.y = y;
        fn.push_left(a, A - L);
        fn.push_left(b, R - B);
        return fn;
    }

    // f(x) = {a(M-x)+y if x<=M | b(x-M)+y if M<=x | for L<=x<=R}
    static LinSlopes vee(V a, V b, V L, V M, V R, V y) {
        assert(a + b >= 0 && L <= R);
        LinSlopes fn;
        if (L <= M && M <= R) {
            if (a < 0) {
                fn.a = fn.b = L, fn.y = a * (M - L) + y;
                fn.push_right(-a, M - L), fn.push_right(+b, R - M);
            } else if (b < 0) {
                fn.a = fn.b = R, fn.y = b * (R - M) + y;
                fn.push_left(+a, M - L), fn.push_left(-b, R - M);
            } else {
                fn.a = a ? M : L, fn.b = b ? M : R, fn.y = y;
                fn.push_left(+a, M - L), fn.push_right(+b, R - M);
            }
        } else if (M < L) {
            if (b >= 0) {
                fn.y = b * (L - M) + y;
                fn.a = L, fn.b = b ? L : R;
                fn.push_right(b, R - L);
            } else {
                fn.y = b * (R - M) + y;
                fn.a = fn.b = R;
                fn.push_left(-b, R - L);
            }
        } else /* R < M */ {
            if (a >= 0) {
                fn.y = a * (M - R) + y;
                fn.a = a ? R : L, fn.b = R;
                fn.push_left(a, R - L);
            } else {
                fn.y = a * (M - L) + y;
                fn.a = fn.b = L;
                fn.push_right(-a, R - L);
            }
        }
        return fn;
    }

    // f(x) := min{y+z=x} a(y) + b(z). Small-to-large merge
    static void minplus(LinSlopes& fn, LinSlopes& gn) {
        fn.a += gn.a, fn.a += gn.b, fn.y += gn.y;
        pq_merge(fn.L, gn.L, fn.Loff, gn.Loff);
        pq_merge(fn.R, gn.R, fn.Roff, gn.Roff);
        gn = LinSlopes();
    }

    // f(x) := g(x+c)
    void shift(V c) { a -= c, b -= c; }

    // f(x) := g(x)+dy
    void add_constant(V dy) { y += dy; }

    // f(x) := min{da+x<=y<=db+x} g(y)
    void range_min(V da, V db) { assert(da <= db), a -= db, b -= da; }

    // f(x) := min{x<=y} g(y). Clears the left heap.
    void suffix_min() {
        while (L.size()) {
            a -= pop_left().second;
        }
        Loff = 0;
    }

    // f(x) := min{y<=x} g(y). Clears the right heap.
    void prefix_min() {
        while (R.size()) {
            b += pop_right().second;
        }
        Roff = 0;
    }

    // f(x) := mx + c + g(x). O(log n) amortized if m always has the same sign.
    void add_linear(V m, V c = 0) {
        add_constant(c);
        auto k = abs(m);
        if (m > 0) {
            push_right(k, b - a), b = a, Roff += k;
        } else if (m < 0) {
            push_left(k, b - a), a = b, Loff += k;
        }
        while (m > 0 && L.size() && L.top().first + Loff <= k) {
            auto [s, len] = pop_left();
            a -= len, b -= s < k ? len : 0;
            push_right(k - s, len);
        }
        while (m < 0 && R.size() && R.top().first + Roff <= k) {
            auto [s, len] = pop_right();
            b += len, a += s < k ? len : 0;
            push_left(k - s, len);
        }
        if (m > 0) {
            Loff -= k;
        } else if (m < 0) {
            Roff -= k;
        }
    }

    V minimum() const { return y; }
    V left_argmin() const { return a; }
    V right_argmin() const { return b; }

    V destructive_query(V x) {
        if (a <= x && x <= b) {
            return y;
        } else if (x < a) {
            V height = y, dx = a - x;
            while (dx > 0 && L.size()) {
                auto [s, len] = pop_left();
                V adv = min(dx, len);
                height += adv * s;
                dx -= adv;
            }
            return dx > 0 ? inf : height;
        } else /* b < x */ {
            V height = y, dx = x - b;
            while (dx > 0 && R.size()) {
                auto [s, len] = pop_right();
                V adv = min(dx, len);
                height += adv * s;
                dx -= adv;
            }
            return dx > 0 ? inf : height;
        }
    }

  private:
    static void pq_merge(min_heap& a, min_heap& b, V& aoff, V& boff) {
        if (a.size() < b.size()) {
            swap(a, b), swap(aoff, boff);
        }
        while (b.size()) {
            auto [s, len] = b.top();
            V ans = 0;
            do {
                ans += b.top().second, b.pop();
            } while (b.size() && b.top().first == s);
            a.emplace(s + boff - aoff, len);
        }
    }
    auto push_left(V slope, V length) {
        if (slope && length) {
            L.emplace(slope - Loff, length);
        }
    }
    auto push_right(V slope, V length) {
        if (slope && length) {
            R.emplace(slope - Roff, length);
        }
    }
    auto pop_left() {
        auto [s, len] = L.top();
        V ans = 0;
        do {
            ans += L.top().second, L.pop();
        } while (L.size() && L.top().first == s);
        return make_pair(s + Loff, ans);
    }
    auto pop_right() {
        auto [s, len] = R.top();
        V ans = 0;
        do {
            ans += R.top().second, R.pop();
        } while (R.size() && R.top().first == s);
        return make_pair(s + Roff, ans);
    }
};
