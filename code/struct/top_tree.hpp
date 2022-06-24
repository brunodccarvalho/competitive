#pragma once

#include <bits/stdc++.h>
using namespace std;

struct top_node {
    static constexpr bool FULL_PUSHUP = false, FULL_PUSHDOWN = false;

    void flip_path() {}

    void pushdown(top_node&, top_node&, top_node&) {}

    void pushup(const top_node&, const top_node&, const top_node&) {}
};

// The heavy trees are node-only trees and the light trees are edge-only trees.
// The edges inside a heavy splay tree are not part of it, they hang on the nodes instead.
// To access the edge nodes during pushup/pushdown set the EDGE flags to true.
template <typename Node>
struct top_tree {
    // Blueprint: pushup(is-node, left-kid, right-kid, other-kid)
    // Blueprint: pushdown(is-node, left-kid, right-kid, other-kid)
    // Kids: {left-heavy, right-heavy, light-kid, mirror-parent, mirror-kid}
    struct TopNode {
        int parent = 0, kids[5] = {};
        int8_t flip = 0;
        Node data;
    };
    static constexpr int8_t LINK = 2, UPPER = 3, LOWER = 4;

    int N, E = 0;
    vector<TopNode> st;
    vector<int> freelist;

    explicit top_tree(int N = 0) : N(N), st(2 * N), freelist(N - 1) {
        for (int i = 0; i < N - 1; i++) {
            freelist[i] = N + i + 1;
        }
    }

    template <typename T>
    explicit top_tree(int N, const vector<T>& arr) : top_tree(N) {
        for (int u = 1; u <= N; u++) {
            st[u].data = Node(true, arr[u]);
        }
    }

    // ***** Node updates
  private:
    void orient(int e, int p) {
        if (st[e].kids[UPPER] != p) {
            swap(st[e].kids[UPPER], st[e].kids[LOWER]);
        }
    }

    void flip(int u) {
        if (u == 0) {
            return;
        }
        auto& [l, r, link, a, b] = st[u].kids;
        swap(l, r);
        swap(a, b);
        st[u].flip ^= 1;
        st[u].data.flip_path();
    }

    void pushdown(int u) {
        if (u == 0) {
            return;
        }
        auto& [l, r, link, a, b] = st[u].kids;
        if (st[u].flip) {
            flip(l);
            flip(r);
            st[u].flip = 0;
        }
        if constexpr (!Node::FULL_PUSHDOWN) {
            st[u].data.pushdown(u <= N, st[l].data, st[r].data, st[link].data);
        } else if (u <= N) {
            int hi = st[a].kids[LINK] || l == 0 ? 0 : a;
            int lo = b == 0 || r == 0 ? 0 : b;
            st[u].data.pushdown(true, st[l].data, st[r].data, //
                                st[link].data, st[hi].data, st[lo].data);
        } else {
            st[u].data.pushdown(false, st[l].data, st[r].data, //
                                st[link].data, st[a].data, st[b].data);
        }
    }

    void pushup(int u) {
        auto& [l, r, link, a, b] = st[u].kids;
        if constexpr (!Node::FULL_PUSHUP) {
            st[u].data.pushup(u <= N, st[l].data, st[r].data, st[link].data);
        } else if (u <= N) {
            int hi = st[a].kids[LINK] || l == 0 ? 0 : a;
            int lo = b == 0 || r == 0 ? 0 : b;
            st[u].data.pushup(true, st[l].data, st[r].data, //
                              st[link].data, st[hi].data, st[lo].data);
        } else {
            st[u].data.pushup(false, st[l].data, st[r].data, //
                              st[link].data, st[a].data, st[b].data);
        }
    }

    template <typename... Vs>
    int new_edge(int upper, int lower, Vs&&... args) {
        int e = freelist[E++];
        st[e].data = Node(false, forward<Vs>(args)...);
        st[e].kids[UPPER] = upper;
        st[e].kids[LOWER] = lower;
        return e;
    }

    void rem_edge(int e) {
        st[e] = TopNode();
        freelist[--E] = e;
    }

    void fixup(int u, int v) {
        if (int p = st[v].parent; p && u != p) {
            pushdown(p), pushdown(v), rotate(v);
        }
    }

    int min_node(int u) {
        while (st[u].kids[0]) {
            u = st[u].kids[0];
            pushdown(u);
        }
        return u;
    }

    // ***** Interface
  public:
    template <typename... Vs>
    bool link(int u, int v, Vs&&... args) {
        reroot(v), access(u);
        if (st[v].parent)
            return false;

        int e = new_edge(u, v, forward<Vs>(args)...);
        st[v].kids[UPPER] = e;
        adopt(e, v, LINK);
        light_insert(u, e);
        pushup(u);
        return true;
    }

    bool cut(int u, int v) {
        reroot(v), access(u), fixup(u, v);
        if (!st[v].parent || v != st[u].kids[0] || st[v].kids[1])
            return false; // u and v are not connected by an edge

        int e = st[u].kids[UPPER];
        st[u].kids[0] = st[u].kids[UPPER] = 0;
        st[v].parent = st[v].kids[LOWER] = 0;
        rem_edge(e), pushup(u), pushup(v);
        return true;
    }

    void reroot(int u) {
        access(u);
        flip(u);
        pushdown(u);
    }

    int findroot(int u) {
        access(u);
        u = min_node(u);
        splay(u);
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

    Node* access_subtree(int u, int v) {
        reroot(v), access(u);
        return &st[u].data;
    }

    Node* access_tree(int u) {
        reroot(u), access(u);
        return &st[u].data;
    }

    Node* access_edge(int u, int v) {
        reroot(v), access(u);
        assert(st[v].parent == u && !st[u].parent && st[u].kids[0] == v);
        int e = st[u].kids[UPPER];
        return &st[e].data;
    }

    // ***** Implementation
  private:
    bool is_root(int u) const {
        auto [l, r, t, a, b] = st[st[u].parent].kids;
        return u != l && u != r;
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
            pushdown(p), pushdown(u), rotate(u), p = g;
        }
        if (p) {
            adopt(p, u, LINK);
            pushdown(p), pushdown(u), pushup(u), pushup(p);
        } else {
            pushdown(u), pushup(u);
        }
    }

    int access(int u) {
        splay(u);
        make_right_light(u);
        int w = u, e = st[u].parent;
        while (e) {
            splay(e);
            int v = st[e].parent;
            splay(v);
            light_splice(v, e);
            make_right_light(v);
            heavy_attach(v, w, e, 1);
            w = v, e = st[v].parent;
        }
        splay(u);
        return w;
    }

    void make_right_light(int u) {
        auto& [l, r, link, a, b] = st[u].kids;
        if (r != 0) {
            light_attach(u, r, b);
            r = b = 0;
            pushup(u);
        }
    }

    void heavy_attach(int u, int v, int e, int8_t side) {
        adopt(u, e, LOWER);
        adopt(u, v, side);
        st[e].kids[LINK] = 0;
        pushup(u);
    }

    void light_attach(int u, int v, int e) {
        orient(e, u);
        adopt(e, v, LINK);
        pushup(e);
        light_insert(u, e);
    }

    void light_insert(int u, int e) {
        int link = st[u].kids[LINK];
        adopt(e, link, 0);
        adopt(e, 0, 1);
        adopt(u, e, LINK);
        pushup(e);
    }

    void light_splice(int v, int e) {
        pushdown(e);
        auto& [l, r, link, a, b] = st[e].kids;
        int root = l ? l : r;
        if (l && r) {
            root = min_node(r);
            st[r].parent = 0;
            splay(root);
            adopt(root, l, 0);
            pushup(root);
        }
        adopt(v, root, LINK);
        st[e].parent = l = r = link = 0;
        // pushup(e);
    }
};

struct top_sum_node {
    static constexpr bool FULL_PUSHUP = false, FULL_PUSHDOWN = false;
    using V = int64_t;
    int path_size = 0;
    int subt_size = 0;
    int virt_size = 0;
    V self = 0;
    V path = 0;
    V subt = 0;
    V virt = 0;
    V lazy_path = 0;
    V lazy_subt = 0;
    V lazy_virt = 0;

    top_sum_node() = default;
    top_sum_node(bool is_node, V v = 0)
        : path_size(is_node), subt_size(is_node), virt_size(is_node), self(v), path(v),
          subt(v), virt(v) {}

    void add_virt(V x) {
        if (subt_size) {
            self += x;
            path += x;
            subt += x * virt_size;
            virt += x * virt_size;
            lazy_virt += x;
        }
    }
    void add_subt(bool is_node, V x) {
        if (subt_size) {
            self += is_node ? x : 0;
            path += x * path_size;
            subt += x * subt_size;
            virt += x * virt_size;
            lazy_subt += x;
        }
    }
    void add_path(V x) {
        if (subt_size) {
            self += x;
            path += x * path_size;
            subt += x * path_size;
            virt += x;
            lazy_path += x;
        }
    }

    void flip_path() {}

    void pushup(bool is_node, const top_sum_node& lhs, const top_sum_node& rhs,
                const top_sum_node& tree) {
        assert(lazy_path == 0);
        assert(lazy_subt == 0);
        assert(lazy_virt == 0);
        path_size = is_node + lhs.path_size + rhs.path_size;
        subt_size = is_node + lhs.subt_size + rhs.subt_size + tree.subt_size;
        virt_size = is_node + tree.subt_size;
        path = self + lhs.path + rhs.path;
        subt = self + lhs.subt + rhs.subt + tree.subt;
        virt = self + tree.subt;
    }

    void pushdown(bool is_node, top_sum_node& lhs, top_sum_node& rhs,
                  top_sum_node& tree) {
        if (lazy_virt) {
            tree.add_subt(!is_node, lazy_virt);
            lazy_virt = 0;
        }
        if (lazy_path) {
            lhs.add_path(lazy_path);
            rhs.add_path(lazy_path);
            lazy_path = 0;
        }
        if (lazy_subt) {
            lhs.add_subt(is_node, lazy_subt);
            rhs.add_subt(is_node, lazy_subt);
            tree.add_subt(!is_node, lazy_subt);
            lazy_subt = 0;
        }
    }
};

struct top_clusters {
    static constexpr bool FULL_PUSHUP = false, FULL_PUSHDOWN = false;

    struct Data {
        int full = 1;
        int lsum = 0;
        int rsum = 0;
        int64_t blob = 0;
        int64_t cold = 0;

        Data() = default;
        Data(int self) : full(self), lsum(self), rsum(self) {}

        static int64_t sq(int64_t x) { return x * x; }

        int64_t rblob() const { return full ? blob : blob + sq(lsum); }
        int64_t lblob() const { return full ? blob : blob + sq(rsum); }
        int64_t query() const { return lblob() + sq(lsum); }

        void flip() { swap(lsum, rsum); }

        void add(Data o) {
            blob += o.blob;
            lsum += o.lsum;
            rsum += o.rsum;
            cold += o.cold;
        }

        void make_edge(Data node) {
            blob = node.lblob();
            lsum = node.lsum;
            rsum = node.lsum;
            cold = sq(node.lsum);
        }

        void make_node(Data edge) {
            blob = full ? edge.blob : edge.blob + edge.cold;
            lsum = full ? 1 + edge.lsum : 0;
            rsum = full ? 1 + edge.rsum : 0;
            cold = 0;
        }

        // [la...ra] [lb...rb] to [la...rb]
        static Data merge(Data a, Data b) {
            Data c;
            c.full = a.full & b.full;
            c.lsum = a.full ? a.lsum + b.lsum : a.lsum;
            c.rsum = b.full ? a.rsum + b.rsum : b.rsum;
            c.blob = a.blob + b.blob + (a.full || b.full ? 0 : sq(a.rsum + b.lsum));
            return c;
        }
    };

    int self = 0;
    Data data;

    top_clusters() = default;
    top_clusters(bool) {}

    int64_t query() const { return data.query(); }

    void flip_path() { data.flip(); }

    void pushup(bool is, const top_clusters& lhs, const top_clusters& rhs,
                const top_clusters& tree) {
        if (is) {
            data.full = self;
            data.make_node(tree.data);
            data = Data::merge(lhs.data, data);
            data = Data::merge(data, rhs.data);
        } else {
            data.make_edge(tree.data);
            data.add(lhs.data);
            data.add(rhs.data);
        }
    }

    void pushdown(bool, top_clusters&, top_clusters&, top_clusters&) {}
};
