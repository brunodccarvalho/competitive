#pragma once

#include "hash.hpp"
#include "struct/pbds.hpp"

struct ett_node_empty {
    void pushdown(bool, bool, ett_node_empty&, ett_node_empty&) {}
    void pushup(bool, bool, ett_node_empty&, ett_node_empty&) {}
};

/**
 * Unrooted euler tour tree (nodes 1-indexed)
 * The splay tree contains three types of nodes: vertex nodes, main edge nodes and
 * secondary edge nodes. The type of the node is specified in the constructor.
 * Values on edges are kept on the main edge nodes only.
 *
 * You must call end_access() after access_subtree() to finish the range splay.
 */
template <typename Node>
struct euler_tour_tree {
    struct ETTNode {
        int parent = 0, kids[2] = {};
        Node node;
    };

    template <typename K, typename V, typename Hash = std::hash<K>>
    using hash_table = gnu::cc_hash_table<K, V, Hash>;

    int N, F = 0;
    vector<ETTNode> st;
    hash_table<pair<int, int>, int> edgemap;
    vector<int> freelist;

    explicit euler_tour_tree(int N = 0) : N(N), st(3 * N + 3), freelist(N) {
        for (int i = 0, f = (N | 1) + 1; i < N; i++) {
            freelist[i] = f + 2 * i;
        }
    }

    template <typename T>
    euler_tour_tree(int N, const vector<T>& arr) : euler_tour_tree(N) {
        for (int u = 1; u <= N; u++) {
            st[u].node = Node(arr[u]);
        }
    }

  protected:
    int get_edge(int u, int v) const {
        auto ituv = edgemap.find(minmax(u, v));
        return ituv == edgemap.end() ? 0 : ituv->second ^ (u > v);
    }

    int add_edge(int u, int v) {
        int uv = freelist[F++], vu = uv ^ 1;
        st[uv] = st[vu] = {};
        edgemap[minmax(u, v)] = uv ^ (u > v);
        return uv;
    }

    void rem_edge(int u, int v) {
        auto ituv = edgemap.find(minmax(u, v));
        freelist[--F] = ituv->second;
        edgemap.erase(minmax(u, v));
    }

    void pushdown(int u) {
        if (u != 0) {
            auto [l, r] = st[u].kids;
            bool is_node = u <= N, is_edge = u > N && !(u & 1);
            st[u].node.pushdown(is_node, is_edge, st[l].node, st[r].node);
        }
    }

    void pushup(int u) {
        auto [l, r] = st[u].kids;
        pushdown(l), pushdown(r);
        bool is_node = u <= N, is_edge = u > N && !(u & 1);
        st[u].node.pushup(is_node, is_edge, st[l].node, st[r].node);
    }

  public:
    bool link(int u, int v) {
        reroot(u), splay(v);
        if (st[u].parent)
            return false;

        int uv = add_edge(u, v), vu = uv ^ 1;
        int r = splay_split<1>(v);
        splay_join(v, splay_join(vu, u, uv), r);
        return true;
    }

    bool cut(int u, int v) {
        int uv = get_edge(u, v), vu = uv ^ 1;
        if (uv == 0)
            return false;

        reroot(v); // v ..A.. vu u ..B.. uv ..C.. --> v ..A.. ..C.. | u ..B..
        splay_join(splay_erase(vu).first, splay_erase(uv).second);
        rem_edge(u, v);
        return true;
    }

    void reroot(int u) { shift_to_front(u); }

    int findroot(int u) {
        splay(u);
        return min_node(u);
    }

    bool conn(int u, int v) {
        if (u == v)
            return true;
        splay(u), splay(v);
        return st[u].parent;
    }

    bool edge_goes_down(int u, int v) {
        int uv = get_edge(u, v), vu = uv ^ 1;
        if (uv == 0)
            return false;

        splay(uv), splay(vu);
        if (st[uv].parent != vu) {
            rotate(uv);
        }
        return uv == st[vu].kids[0];
    }

    Node* access_node(int u) {
        splay(u);
        return &st[u].node;
    }

    Node* access_edge(int u, int v) {
        int uv = get_edge(min(u, v), max(u, v));
        assert(uv > 0), splay(uv);
        return &st[uv].node;
    }

    Node* access_tree(int u) {
        splay(u);
        return &st[u].node;
    }

    // access the subtree of u assuming v is the root of u's tree. Edge uv must exist.
    Node* access_subtree(int u, int v) {
        reroot(v);
        int uv = get_edge(u, v), vu = uv ^ 1;
        assert(uv > 0);
        int s = waiting = range_splay(vu, uv);
        return &st[s].node;
    }

    void end_access() {
        if (waiting) {
            splay(waiting), waiting = 0;
        }
    }

  protected:
    int waiting = 0;

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

    int range_splay(int u, int v) {
        splay(v), splay(u);
        if (st[v].parent != u) {
            rotate(v);
        }
        int s = st[v].kids[0];
        pushdown(s), pushup(s);
        return s;
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
            return l ^ r;
        assert(is_root(l) && is_root(r));
        l = max_node(l);
        splay(l);
        adopt(l, r, 1);
        pushup(l);
        return l;
    }

    auto splay_erase(int u) {
        int r = splay_split<1>(u), l = splay_split<0>(u);
        return make_pair(l, r);
    }

    int splay_join(int l, int m, int r) { return splay_join(splay_join(l, m), r); }
    void shift_to_front(int u) { splay_join(u, splay_split<0>(u)), splay(u); }
    void shift_to_back(int u) { splay_join(splay_split<1>(u), u), splay(u); }
};

template <typename T>
struct ett_node_sum {
    int subt_size = 0;
    T self = 0;
    T subt = 0;
    T lazy = 0;

    T subtree() const { return subt; }
    int subtree_size() const { return subt_size; }

    void add_subtree(T plus) { lazy += plus; }

    void pushdown(bool is_node, bool, ett_node_sum& lhs, ett_node_sum& rhs) {
        if (lazy) {
            lhs.lazy += lazy;
            rhs.lazy += lazy;
            if (is_node) {
                self += lazy;
            }
            subt += lazy * subt_size;
            lazy = 0;
        }
    }

    void pushup(bool is_node, bool, const ett_node_sum& lhs, const ett_node_sum& rhs) {
        subt_size = is_node + lhs.subt_size + rhs.subt_size;
        subt = self + lhs.subt + rhs.subt;
    }
};
