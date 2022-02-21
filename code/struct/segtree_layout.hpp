#pragma once

#include <bits/stdc++.h>
using namespace std;

// Source: https://github.com/ecnerwala/cp-book/blob/master/src/seg_tree.hpp

struct segpoint {
    int a;
    segpoint() : a(0) {}
    segpoint(int a) : a(a) {}

    explicit operator bool() const noexcept { return a; }
    operator int() const noexcept { return a; }

    segpoint kid(int8_t side) const { return (a << 1) | side; }
    segpoint operator[](int8_t side) const { return (a << 1) | side; }
    segpoint parent() const { return a >> 1; }

    segpoint& operator++() { return ++a, *this; }
    segpoint& operator--() { return --a, *this; }
    segpoint operator++(int) { return segpoint(a++); }
    segpoint operator--(int) { return segpoint(a--); }

    // Loop up through parent path of this point including itself
    template <typename Fn>
    void for_each(Fn&& fn) const {
        for (int i = a; i > 0; i >>= 1)
            fn(segpoint(i));
    }
    // Loop down through parent path of this point not including itself
    template <typename Fn>
    void for_parents_down(Fn&& fn) const {
        for (int b = log2(a); b > 0; b--)
            fn(segpoint(a >> b));
    }
    // Loop up through parent path of this point not including itself
    template <typename Fn>
    void for_parents_up(Fn&& fn) const {
        for (int i = a >> 1; i > 0; i >>= 1)
            fn(segpoint(i));
    }

    static int logbit(int x) { return x ? (8 * sizeof(int) - 1 - __builtin_clz(x)) : -1; }
    static int next_two(int x) {
        return 1 << (x ? (8 * sizeof(int) - __builtin_clz(x - 1)) : 0);
    }

    friend string to_string(segpoint x) { return to_string(x.a); }
    friend ostream& operator<<(ostream& out, segpoint x) { return out << x; }
};

struct segrange {
    int a, b;
    segrange() : a(1), b(1) {}
    segrange(int a, int b) : a(a), b(b) { assert(1 <= a && a <= b && b <= 2 * a); }
    explicit segrange(array<int, 2> x) : segrange(x[0], x[1]) {}

    operator array<int, 2>() const noexcept { return {a, b}; }

    int operator[](int8_t side) const { return side ? b : a; }

    // Loop over the segnodes in this range upwards and inwards
    template <typename Fn>
    void for_each(Fn&& fn) const {
        for (int x = a, y = b; x < y; x >>= 1, y >>= 1) {
            if (x & 1)
                fn(segpoint(x++));
            if (y & 1)
                fn(segpoint(--y));
        }
    }
    // Loop over the segnodes in this range upwards and inwards
    template <typename Fn>
    void for_each_with_side(Fn&& fn) const {
        for (int x = a, y = b; x < y; x >>= 1, y >>= 1) {
            if (x & 1)
                fn(segpoint(x++), 0);
            if (y & 1)
                fn(segpoint(--y), 1);
        }
    }
    // Loop over the segnodes in this range from left to right
    template <typename Fn>
    void for_each_l_to_r(Fn&& fn) const {
        int lca_depth = segpoint::logbit((a - 1) ^ b);
        int mask = (1 << lca_depth) - 1;
        for (int v = (-a) & mask; v; v &= v - 1) {
            int i = __builtin_ctz(v);
            fn(segpoint(((a - 1) >> i) + 1));
        }
        for (int v = b & mask; v;) {
            int i = segpoint::logbit(v);
            fn(segpoint((b >> i) - 1));
            v ^= 1 << i;
        }
    }
    // Loop over the segnodes in this range from right to left
    template <typename Fn>
    void for_each_r_to_l(Fn&& fn) const {
        int lca_depth = segpoint::logbit((a - 1) ^ b);
        int mask = (1 << lca_depth) - 1;
        for (int v = b & mask; v; v &= v - 1) {
            int i = __builtin_ctz(v);
            fn(segpoint((b >> i) - 1));
        }
        for (int v = (-a) & mask; v;) {
            int i = segpoint::logbit(v);
            fn(segpoint(((a - 1) >> i) + 1));
            v ^= (1 << i);
        }
    }

    friend string to_string(segrange x) {
        return '[' + to_string(x.a) + ',' + to_string(x.b) + ')';
    }
    friend ostream& operator<<(ostream& out, segrange x) { return out << x; }
};

struct standard_layout {
    using interval = array<int, 2>;

    int N, S;
    standard_layout() : N(0), S(0) {}
    standard_layout(int N) : N(N), S(N ? segpoint::next_two(N) : 0) {}

    // Get the leaf (a) in seg space representing the index [i] in array space
    segpoint point(int i) const {
        assert(0 <= i && i < N);
        i += S;
        return segpoint(i >= 2 * N ? i - N : i);
    }

    // Get the range (a,b) in seg space representing the interval [l,r) in array space
    segrange range(int l, int r) const {
        assert(0 <= l && l <= r && r <= N);
        l += S, r += S;
        l = l > 2 * N ? 2 * (l - N) : l; // keep r on the lower level
        r = r > 2 * N ? 2 * (r - N) : r; // keep r on the lower level
        return N ? segrange(l, r) : segrange();
    }

    // Get the index [i] in array space represented by the leaf index (a) in seg space
    int get_leaf_index(segpoint a) const {
        assert(N <= a && a < 2 * N);
        return (a < S ? a + N : int(a)) - S;
    }

    // Get the interval [l,r) in array space represented by the range (a) in seg space
    interval get_node_bounds(segpoint a) const {
        assert(1 <= a && a < 2 * N);
        int l = __builtin_clz(a) - __builtin_clz(2 * N - 1);
        int x = a << l, y = (a + 1) << l;
        assert(S <= x && x < y && y <= 2 * S);
        x = (x >= 2 * N ? (x >> 1) + N : x) - S;
        y = (y >= 2 * N ? (y >> 1) + N : y) - S;
        return {x, y};
    }

    // Get the middle of the interval [l,r) in array space represented by (a) in seg space
    int get_node_split(segpoint a) const {
        assert(1 <= a && a < 2 * N);
        int l = __builtin_clz(2 * a + 1) - __builtin_clz(2 * N - 1);
        int x = (2 * a + 1) << l;
        assert(S <= x && x < 2 * S);
        int m = (x >= 2 * N ? (x >> 1) + N : x) - S;
        return m;
    }

    // Get the size of the interval [l,r) in array space represented by (a) in seg space
    int get_node_size(int a) const {
        auto [x, y] = get_node_bounds(a);
        return y - x;
    }
};
