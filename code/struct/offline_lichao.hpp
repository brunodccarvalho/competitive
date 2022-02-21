#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T>
struct Affine {
    static inline Affine<T> inf = {T(0), numeric_limits<T>::max() / 2};
    T a = 0, b = 0; // ax + b
    T operator()(T x) { return a * x + b; }
};

/**
 * Returns minimum on point. Answers queries offline.
 *
 * Usage:
 *   vector<T> xs;
 *   ... add query x to xs ...
 *   offline_lichao<T, ...> st(xs, infinity);
 *
 *   st.add_segment(l, r, fn);              l/r do not need to be in xs
 *   st.add_line(fn);
 *   auto y = st.query_min_point(x);        x needs to be in xs
 */
template <typename T, typename Func> // Minimum on point
struct offline_lichao {
    vector<Func> tree;
    vector<T> xs;

    offline_lichao() = default;
    offline_lichao(int L, int R, Func fn) { assign(L, R, fn); }
    offline_lichao(vector<T> xs, Func fn) { assign(move(xs), fn); }

    int argsize() const { return xs.size(); }

    void assign(T L, T R, Func fn) {
        vector<T> xs(R - L);
        iota(begin(xs), end(xs), L);
        assign(move(xs), fn);
    }

    void assign(vector<T> query_xset, Func fn) {
        xs = move(query_xset);
        assert(is_sorted(begin(xs), end(xs)) && unique(begin(xs), end(xs)) == end(xs));
        int N = argsize();
        int log = N > 1 ? 8 * sizeof(N) - __builtin_clz(N - 1) : 0;
        int size = 1 << log;
        tree.assign(2 * size, fn);
    }

    void add_segment(T L, T R, Func fn) {
        int l = lower_bound(begin(xs), end(xs), L) - begin(xs);
        int r = lower_bound(begin(xs), end(xs), R) - begin(xs);
        if (l < r) {
            update_range(1, 0, argsize(), l, r, fn);
        }
    }

    void add_line(Func fn) { inner_range_update(1, 0, argsize(), fn); }

    auto query_min_point(T x) {
        int i = lower_bound(begin(xs), end(xs), x) - begin(xs); // ! UNCHECKED
        return inner_query(i);
    }

  private:
    void inner_range_update(int i, int l, int r, Func fn) {
        while (true) {
            int m = (l + r) / 2;
            bool dl = fn(xs[l]) < tree[i](xs[l]);
            bool dr = fn(xs[r - 1]) < tree[i](xs[r - 1]);
            if (dl == dr) {
                if (dl) {
                    tree[i] = fn;
                }
                return;
            }
            bool dm = fn(xs[m]) < tree[i](xs[m]);
            if (dm) {
                swap(tree[i], fn);
            }
            if (dl != dm) {
                r = m, i = i << 1;
            } else {
                l = m, i = i << 1 | 1;
            }
        }
    }

    void update_range(int i, int l, int r, int ql, int qr, Func fn) {
        if (ql <= l && r <= qr) {
            return inner_range_update(i, l, r, fn);
        }
        int m = (l + r) / 2;
        if (qr <= m) {
            return update_range(i << 1, l, m, ql, qr, fn);
        } else if (m <= ql) {
            return update_range(i << 1 | 1, m, r, ql, qr, fn);
        } else {
            update_range(i << 1, l, m, ql, m, fn);
            update_range(i << 1 | 1, m, r, m, qr, fn);
        }
    }

    auto inner_query(int p) {
        int i = 1, l = 0, r = argsize();
        auto ymin = tree[i](xs[p]);
        while (r - l > 1) {
            if (int m = (l + r) / 2; p < m) {
                r = m, i = i << 1;
            } else {
                l = m, i = i << 1 | 1;
            }
            ymin = min(ymin, tree[i](xs[p]));
        }
        return ymin;
    }
};
