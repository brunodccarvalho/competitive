#pragma once

#include <bits/stdc++.h>
using namespace std;

// Piecewise-linear convex function with slope changing points stored in heaps
// Supports pointwise addition of functions but not min-plus convolution
// Source: https://github.com/saketh-are/algo-lib
template <typename T>
struct LinPoints {
    static inline constexpr T MIN = numeric_limits<T>::lowest() / 4;
    static inline constexpr T MAX = numeric_limits<T>::max() / 4;
    template <typename U>
    using max_heap = priority_queue<U, vector<U>, less<U>>;
    template <typename U>
    using min_heap = priority_queue<U, vector<U>, greater<U>>;

    T y0, left_offset = 0, right_offset = 0;
    max_heap<pair<T, T>> left;
    min_heap<pair<T, T>> right;

    explicit LinPoints(T y0 = 0) : y0(y0) {}

    int size() const { return left.size() + right.size(); }

    T minimum() const { return y0; }
    T left_arg_min() const {
        return left.empty() ? MIN : (left.top().first + left_offset);
    }
    T right_arg_min() const {
        return right.empty() ? MAX : (right.top().first + right_offset);
    }

    auto destructive_query(T x) {
        auto c = y0;
        auto L = left_arg_min();
        auto R = right_arg_min();
        if (x < L) {
            while (left.size() && left.top().first + left_offset > x) {
                auto [lx, slope] = left.top();
                if (slope >= MAX / 2)
                    return MAX;
                left.pop(), lx += left_offset;
                c += slope * (lx - x);
            }
        } else if (R < x) {
            while (right.size() && right.top().first + right_offset < x) {
                auto [rx, slope] = right.top();
                if (slope >= MAX / 2)
                    return MAX;
                right.pop(), rx += right_offset;
                c += slope * (x - rx);
            }
        }
        return c;
    }

    // __ : f(x) = b
    static LinPoints constant(T b) { return LinPoints(b); }

    // \__/ : f(x) = max(0, c(x0-x), c(x-x1)) + b, positive c
    static LinPoints range(T c, T x0, T x1, T b = 0) {
        LinPoints f(b);
        assert(c > 0 && x0 <= x1);
        f.right.emplace(x1, c);
        f.left.emplace(x0, c);
        return f;
    }

    // \_ or _/ : f(x) = max(0, c * (x - x0)) + b
    static LinPoints slope(T c, T x0, T b = 0) {
        LinPoints f(b);
        if (c < 0) {
            f.left.emplace(x0, -c);
        } else if (c > 0) {
            f.right.emplace(x0, c);
        }
        return f;
    }

    // \/ : y = c * abs(x - x0) + b, positive c
    static LinPoints abs(T c, T x0, T b = 0) {
        LinPoints f(b);
        assert(c > 0);
        f.left.emplace(x0, c);
        f.right.emplace(x0, c);
        return f;
    }

    // f'(x) = f(x + c)
    auto& shift(int c) {
        left_offset -= c;
        right_offset -= c;
        return *this;
    }

    // \_/ => \__ : f'(x) = min_{y <= x} f(y)
    auto& prefix_min() {
        right = min_heap<pair<T, T>>();
        return *this;
    }

    // \_/ => \__ : f'(x) = min_{y >= x} f(y)
    auto& suffix_min() {
        left = max_heap<pair<T, T>>();
        return *this;
    }

    // \_/ => \___/ : f'(x) = min_{dx in [xl, xr]} f(x + dx) */
    auto& range_min(T xl, T xr) {
        assert(xl <= xr);
        if (xl > 0) {
            shift(xl), xr -= xl, xl = 0;
        }
        if (xr < 0) {
            shift(xr), xl -= xr, xr = 0;
        }
        left_offset -= xr;
        right_offset -= xl;
        return *this;
    }

    auto& add_constant(T dy) {
        y0 += dy;
        return *this;
    }

    auto& add(LinPoints&& o) {
        y0 += o.y0;

        while (!o.left.empty()) {
            auto [x, beta_change] = o.left.top();
            x += o.left_offset;
            o.left.pop();

            if (x <= right_arg_min()) {
                left.emplace(x - left_offset, beta_change);
            } else {
                T x0 = right_arg_min();
                y0 += (x - x0) * beta_change;
                right.emplace(x - right_offset, beta_change);
                T beta = beta_change;
                while (beta > 0) {
                    T next_change = right.top().second;
                    right.pop();
                    if (next_change >= beta) {
                        left.emplace(x0 - left_offset, beta);
                        if (next_change > beta) {
                            right.emplace(x0 - right_offset, next_change - beta);
                        }
                        beta = 0;
                    } else {
                        beta -= next_change;
                        y0 -= beta * (right_arg_min() - x0);
                        x0 = right_arg_min();
                    }
                }
            }
        }

        while (!o.right.empty()) {
            auto [x, beta_change] = o.right.top();
            x += o.right_offset;
            o.right.pop();

            if (x >= left_arg_min()) {
                right.emplace(x - right_offset, beta_change);
            } else {
                T x0 = left_arg_min();
                y0 += (x0 - x) * beta_change;
                left.emplace(x - left_offset, beta_change);
                T beta = beta_change;
                while (beta > 0) {
                    T next_change = left.top().second;
                    left.pop();
                    if (next_change >= beta) {
                        right.emplace(x0 - right_offset, beta);
                        if (next_change > beta) {
                            left.emplace(x0 - left_offset, next_change - beta);
                        }
                        beta = 0;
                    } else {
                        beta -= next_change;
                        y0 -= beta * (x0 - left_arg_min());
                        x0 = left_arg_min();
                    }
                }
            }
        }

        return *this;
    }

    LinPoints& add(const LinPoints& o) {
        LinPoints f(o);
        return add(move(f));
    }

    friend LinPoints operator+(LinPoints a, const LinPoints& b) { return a.add(b); }

    auto destructive_format() {
        set<pair<T, T>> pts;
        T L = left_arg_min(), R = right_arg_min();
        pts.emplace(L, y0);
        pts.emplace(R, y0);
        T cL = 0, cR = 0, vL = y0, vR = y0;
        while (!left.empty()) {
            auto [l, c] = left.top();
            left.pop(), l += left_offset;
            vL += (L - l) * cL, cL += c;
            pts.emplace(l, vL);
            if (c >= MAX / 2)
                break;
        }
        while (!right.empty()) {
            auto [r, c] = right.top();
            right.pop(), r += right_offset;
            vR += (r - R) * cR, cR += c;
            pts.emplace(r, vR);
            if (c >= MAX / 2)
                break;
        }
        return to_string(pts);
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
    // valley is [a,b], value there is y, Loff and Roff are lazily added to heaps

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

    // f(x) = {a(L-x)+y for L<=x<=A, y for A<=x<=B, b(R-x)+y for B<=x<=R | for L<=x<=R}
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
            a.push({s + boff - aoff, len}), b.pop();
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
        return L.pop(), make_pair(s + Loff, len);
    }
    auto pop_right() {
        auto [s, len] = R.top();
        return R.pop(), make_pair(s + Roff, len);
    }
};
