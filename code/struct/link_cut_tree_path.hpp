#pragma once

#include <bits/stdc++.h>
using namespace std;

struct lct_node_path_empty {
    void flip_path() {}
    void pushdown(lct_node_path_empty&, lct_node_path_empty&) {}
    void pushup(const lct_node_path_empty&, const lct_node_path_empty&) {}
};

/**
 * Unrooted link cut tree: path queries + path/point updates (nodes 1-indexed)
 */
template <typename Node>
struct link_cut_tree_path {
    struct LCTNode {
        int parent = 0, kids[2] = {};
        int8_t flip = 0; // splay tree is flipped due to reroot
        Node data;
    };

    vector<LCTNode> st;

    explicit link_cut_tree_path(int N = 0) : st(N + 1) {}

    template <typename T>
    link_cut_tree_path(int N, const vector<T>& arr) : st(N + 1) {
        for (int u = 1; u <= N; u++) {
            st[u].data = Node(arr[u]);
        }
    }

    // ***** Node updates
  protected:
    void flip(int u) {
        if (u == 0) {
            return;
        }
        auto& [l, r] = st[u].kids;
        swap(l, r);
        st[u].flip ^= 1;
        st[u].data.flip_path();
    }

    void pushdown(int u) {
        if (u != 0) {
            auto& [l, r] = st[u].kids;
            if (st[u].flip) {
                flip(l);
                flip(r);
                st[u].flip = 0;
            }
            st[u].data.pushdown(st[l].data, st[r].data);
        }
    }

    void pushup(int u) {
        auto [l, r] = st[u].kids;
        st[u].data.pushup(st[l].data, st[r].data);
    }

    // ***** Interface
  public:
    bool link(int u, int v) {
        reroot(u), access(v); // no way to detect cycles without doing extra work
        if (st[u].parent)
            return false;
        st[u].parent = v;
        return true;
    }

    bool cut(int u, int v) {
        reroot(u), access(v);
        if (!st[u].parent || st[u].kids[1] || u != st[v].kids[0])
            return false;
        st[u].parent = st[v].kids[0] = 0;
        return true;
    }

    void reroot(int u) {
        access(u);
        flip(u);
    }

    int findroot(int u) {
        access(u);
        while (st[u].kids[0])
            u = st[u].kids[0], pushdown(u);
        return u;
    }

    int lca(int u, int v) {
        if (u == v)
            return u;
        access(u), v = access(v);
        return st[u].parent ? v : 0;
    }

    bool conn(int u, int v) { return lca(u, v) != 0; }

    Node* access_node(int u) {
        access(u);
        return &st[u].data;
    }

    Node* access_path(int u, int v) {
        reroot(v), access(u);
        return &st[u].data;
    }

  protected:
    bool is_root(int u) const {
        return st[st[u].parent].kids[0] != u && st[st[u].parent].kids[1] != u;
    }

    void adopt(int parent, int child, int8_t side) {
        if (side >= 0)
            st[parent].kids[side] = child;
        if (child)
            st[child].parent = parent;
    }

    void rotate(int u) {
        int p = st[u].parent, g = st[p].parent;
        bool uside = u == st[p].kids[1];
        adopt(p, st[u].kids[!uside], uside);
        adopt(g, u, !is_root(p) ? p == st[g].kids[1] : -1);
        adopt(u, p, !uside);
        pushup(p);
    }

    void splay(int u) {
        int p = st[u].parent, g = st[p].parent;
        while (!is_root(u) && !is_root(p)) {
            pushdown(g), pushdown(p), pushdown(u);
            bool zigzig = (u == st[p].kids[1]) == (p == st[g].kids[1]);
            rotate(zigzig ? p : u), rotate(u);
            p = st[u].parent, g = st[p].parent;
        }
        if (!is_root(u)) {
            pushdown(p), pushdown(u), rotate(u);
        }
        pushdown(u), pushup(u);
    }

    int access(int u) {
        int last = 0, v = u;
        do {
            splay(v);
            st[v].kids[1] = last;
            last = v, v = st[v].parent;
        } while (v);
        splay(u);
        return last;
    }
};

/**
 * Query for sum on path
 * Support += on point and path
 */
struct lct_node_path_sum {
    int path_size = 0;
    int64_t self = 0;
    int64_t path = 0;
    int64_t lazy = 0;

    lct_node_path_sum() = default;
    lct_node_path_sum(int64_t v) : path_size(1), self(v), path(v) {}

    int64_t path_sum() { return path; }

    void add_path(int64_t plus) {
        if (path_size) {
            self += plus;
            path += plus * path_size;
            lazy += plus;
        }
    }

    void flip_path() {}

    void pushdown(lct_node_path_sum& lhs, lct_node_path_sum& rhs) {
        if (lazy) {
            lhs.add_path(lazy);
            rhs.add_path(lazy);
            lazy = 0;
        }
    }

    void pushup(const lct_node_path_sum& lhs, const lct_node_path_sum& rhs) {
        path_size = 1 + lhs.path_size + rhs.path_size;
        path = self + lhs.path + rhs.path;
    }
};

/**
 * Query for maximum on path
 * Support += on point and path
 */
struct lct_node_path_max {
    int path_size = 0;
    int64_t self = 0;
    int64_t path = 0;
    int64_t lazy = 0;

    lct_node_path_max() = default;
    lct_node_path_max(int64_t v) : path_size(1), self(v), path(v) {}

    int64_t path_max() { return path; }

    void add_path(int64_t plus) {
        if (path_size) {
            self += plus;
            path += plus;
            lazy += plus;
        }
    }

    void flip_path() {}

    void pushdown(lct_node_path_max& lhs, lct_node_path_max& rhs) {
        if (lazy) {
            lhs.add_path(lazy);
            rhs.add_path(lazy);
            lazy = 0;
        }
    }

    void pushup(const lct_node_path_max& lhs, const lct_node_path_max& rhs) {
        path_size = 1 + lhs.path_size + rhs.path_size;
        path = max(self, max(lhs.path, rhs.path));
    }
};

/**
 * Query for minimum on path
 * Support += on point and path
 */
struct lct_node_path_min {
    int path_size = 0;
    int64_t self = 0;
    int64_t path = 0;
    int64_t lazy = 0;

    lct_node_path_min() = default;
    lct_node_path_min(int64_t v) : path_size(1), self(v), path(v) {}

    int64_t path_min() { return path; }

    void add_path(int64_t plus) {
        if (path_size) {
            self += plus;
            path += plus;
            lazy += plus;
        }
    }

    void flip_path() {}

    void pushdown(lct_node_path_min& lhs, lct_node_path_min& rhs) {
        if (lazy) {
            lhs.add_path(lazy);
            rhs.add_path(lazy);
            lazy = 0;
        }
    }

    void pushup(const lct_node_path_min& lhs, const lct_node_path_min& rhs) {
        path_size = 1 + lhs.path_size + rhs.path_size;
        path = min(self, min(lhs.path, rhs.path));
    }
};

/**
 * Query for (u,v) returns composition of affine functions f[u],...,f[v] in two orders:
 * - u<-v order: f[u](...(f[u](x)))
 * - v<-u order: f[v](...(f[v](x)))
 * Use the returned data to eval directly for a value, or store the data somewhere.
 * Support := on point
 */
struct lct_node_path_affine {
    using num = int64_t;
    using Data = array<num, 2>;
    Data self = {};
    Data path[2] = {};

    void set(Data fn) { self = path[0] = path[1] = fn; }

    num eval_uv(num x) const { return path[0][0] * x + path[0][1]; }
    num eval_vu(num x) const { return path[1][0] * x + path[1][1]; }
    Data combine(Data a, Data b) { return Data{a[0] * b[0], a[0] * b[1] + a[1]}; }

    void flip_path() { swap(path[0], path[1]); }

    void pushdown(lct_node_path_affine&, lct_node_path_affine&) {}

    void pushup(const lct_node_path_affine& lhs, const lct_node_path_affine& rhs) {
        path[0] = combine(combine(lhs.path[0], self), rhs.path[0]); // lhs(self(rhs(x)))
        path[1] = combine(combine(rhs.path[1], self), lhs.path[1]); // rhs(self(lhs(x)))
    }
};

/**
 * Query returns gcd of all values on path
 * Support += on point and range
 */
struct lct_node_path_gcd {
    int path_size = 0;
    int64_t self = 0;
    int64_t diff = 0;
    int64_t path = 0;
    int64_t lazy = 0;

    lct_node_path_gcd() = default;
    lct_node_path_gcd(int64_t v) : path_size(1), self(v), path(v) {}

    int64_t path_gcd() { return path; }

    void add_path(int64_t plus) {
        if (path_size) {
            self += plus;
            path = gcd(self, diff);
            lazy += plus;
        }
    }

    void flip_path() {}

    void pushdown(lct_node_path_gcd& lhs, lct_node_path_gcd& rhs) {
        if (lazy) {
            lhs.add_path(lazy);
            rhs.add_path(lazy);
            lazy = 0;
        }
    }

    void pushup(const lct_node_path_gcd& lhs, const lct_node_path_gcd& rhs) {
        path_size = 1 + lhs.path_size + rhs.path_size;
        diff = gcd(gcd(lhs.diff, rhs.diff), lhs.self - self);
        path = gcd(self, diff);
    }
};
