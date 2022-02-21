#pragma once

#include <bits/stdc++.h>
using namespace std;

struct dft_node_empty {
    void pushdown(bool, dft_node_empty&, dft_node_empty&) {}
    void pushup(bool, const dft_node_empty&, const dft_node_empty&) {}
};

/**
 * A different way to implement euler tour trees without support for rerooting.
 * The splay tree contains two types of nodes: primary vertex nodes (firsts) and secondary
 * vertex nodes (lasts). Edges are not represented. The pushdown/pushup functions take a
 * bool parameter indicating the type of node.
 *
 * The paper showed this data structure supported more stuff, but I haven't implemented
 * them yet.
 *
 * You must call end_access() after access_subtree() to finish the range splay.
 */
template <typename Node>
struct depth_first_tree {
    struct DFTNode {
        int parent = 0, kids[2] = {};
        int splay_size = 1;
        Node node;
    };

    vector<DFTNode> st;

    explicit depth_first_tree(int N = 0) : st(2 * N + 1) {
        st[0].splay_size = 0;
        for (int u = 1; u <= N; u++)
            splay_join(first(u), last(u));
    }

  private:
    void pushdown(int u) {
        if (u != 0) {
            auto [l, r] = st[u].kids;
            st[u].node.pushdown(u & 1, st[l].node, st[r].node);
        }
    }

    void pushup(int u) {
        auto [l, r] = st[u].kids;
        pushdown(l), pushdown(r);
        st[u].splay_size = 1 + st[l].splay_size + st[r].splay_size;
        st[u].node.pushup(u & 1, st[l].node, st[r].node);
    }

  public:
    void link(int u, int v) {
        splay(first(u));
        assert(first(u) == min_node(first(u))); // u is root of its represented tree
        int m = splay_split<1>(first(v));       // ...[v0] | ...v1... | [u0]...u1
        splay_join(first(v), first(u), m);      // ...v0u0...u1...v1...
    }

    void cut(int u) {
        auto [l, r] = splice(u);
        splay_join(l, r);
    }

    bool conn(int u, int v) {
        if (u == v)
            return true;
        splay(first(u)), splay(first(v));
        return !is_root(first(u));
    }

    int findroot(int u) {
        splay(first(u));
        return rep(min_node(first(u)));
    }

    bool is_descendant(int u, int a) {
        if (u == a)
            return true;
        if (!conn(u, a))
            return false;
        int fu = order_of_node(first(u));
        return order_of_node(first(a)) < fu && fu < order_of_node(last(a));
    }

    Node* access_node(int u) {
        splay(first(u));
        return &st[first(u)].node;
    }

    Node* access_subtree(int u) {
        int s = waiting = range_splay(prev(first(u)), last(u));
        return &st[s].node;
    }

    void end_access() {
        if (waiting) {
            splay(waiting), waiting = 0;
        }
    }

  private:
    int waiting = 0;

    static int rep(int u) { return (u + 1) >> 1; }
    static int first(int n) { return 2 * n - 1; }
    static int last(int n) { return 2 * n; }

    bool is_root(int u) const { return !st[u].parent; }
    bool is_left(int u) const { return st[u].parent && u == st[st[u].parent].kids[0]; }
    bool is_right(int u) const { return st[u].parent && u == st[st[u].parent].kids[1]; }
    int min_node(int u) const {
        while (st[u].kids[0])
            u = st[u].kids[0];
        return u;
    }
    int max_node(int u) const {
        while (st[u].kids[1])
            u = st[u].kids[1];
        return u;
    }
    int prev(int u) {
        if (st[u].kids[0]) {
            return max_node(st[u].kids[0]);
        } else {
            while (is_left(u))
                u = st[u].parent;
            return st[u].parent;
        }
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
        adopt(g, u, g ? p == st[g].kids[1] : -1);
        adopt(u, p, !uside);
        pushup(p);
    }

    void splay(int u) {
        int p = st[u].parent, g = st[p].parent;
        while (p && g) {
            pushdown(g), pushdown(p), pushdown(u);
            bool zigzig = (u == st[p].kids[1]) == (p == st[g].kids[1]);
            rotate(zigzig ? p : u), rotate(u);
            p = st[u].parent, g = st[p].parent;
        }
        if (p) {
            pushdown(p), pushdown(u), rotate(u);
        }
        pushdown(u), pushup(u);
    }

    // splay the range between u and v (non-inclusive), guaranted to not be empty
    int range_splay(int u, int v) {
        if (u == 0)
            return splay(v), st[v].kids[0];
        if (v == 0)
            return splay(u), st[u].kids[1];

        splay(v), splay(u);
        if (st[v].parent != u) {
            rotate(v);
        }
        int s = st[v].kids[0];
        assert(s != 0);
        pushdown(s), pushup(s);
        return s;
    }

    int find_by_order(int u, int order) const {
        if (order >= st[u].splay_size)
            return 0;
        while (true) {
            int v = st[u].kids[0];
            if (order == st[v].splay_size)
                return u;
            if (order < st[v].splay_size) {
                u = v;
            } else {
                order -= st[v].splay_size;
                u = st[u].kids[1];
            }
        }
    }

    int order_of_node(int u) const {
        int order = st[st[u].kids[0]].splay_size;
        while (st[u].parent) {
            if (is_right(u))
                order += st[st[u].parent].kids[0] + 1;
            u = st[u].parent;
        }
        return order;
    }

    template <bool after>
    int splay_split(int u) {
        splay(u);
        int v = st[u].kids[after];
        st[v].parent = st[u].kids[after] = 0;
        pushup(u);
        return v;
    }

    int splay_join(int l, int r) {
        if (l == 0 || r == 0)
            return l ? l : r;
        assert(is_root(l) && is_root(r));
        l = max_node(l);
        splay(l);
        adopt(l, r, 1);
        pushup(l);
        return l;
    }

    int splay_join(int l, int m, int r) { return splay_join(splay_join(l, m), r); }

    auto splice(int u) {
        int r = splay_split<1>(last(u)), l = splay_split<0>(first(u));
        return make_pair(l, r); // first(u) is the root
    }
};

/**
 * Maintain sum and size of subtrees
 */
template <typename T>
struct dft_node_sum {
    int subt_size = 0;
    T self = 0;
    T subt = 0;
    T lazy = 0;

    int subtree_size() const { return subt_size; }
    T subtree() const { return subt; }

    void add_subtree(T plus) { lazy += plus; }

    void pushdown(bool is_main, dft_node_sum& lhs, dft_node_sum& rhs) {
        if (lazy) {
            lhs.lazy += lazy;
            rhs.lazy += lazy;
            if (is_main)
                self += lazy;
            subt += lazy * subt_size;
            lazy = 0;
        }
    }

    void pushup(bool is_main, const dft_node_sum& lhs, const dft_node_sum& rhs) {
        subt_size = is_main + lhs.subt_size + rhs.subt_size;
        subt = self + lhs.subt + rhs.subt;
    }
};
