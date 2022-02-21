#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename Node>
struct link_cut_tree_edge_path {
    struct LCTNode {
        int parent = 0, kids[2] = {};
        int8_t flip = 0; // splay tree is flipped due to reroot
        Node node;
    };

    vector<LCTNode> st;
    vector<int> freelist;
    int F = 0;

    explicit link_cut_tree_edge_path(int N = 0) : st(2 * N), freelist(N - 1) {
        for (int i = 0; i < N - 1; i++) {
            freelist[i] = N + i + 1;
        }
    }

    template <typename T>
    link_cut_tree_edge_path(int N, const vector<T>& arr) : link_cut_tree_edge_path(N) {
        for (int u = 1; u <= N; u++) {
            st[u].node = Node(arr[u]);
        }
    }

    int num_nodes() { return st.size() / 2; }

    // ***** Node updates
  protected:
    void pushdown(int u) {
        if (u != 0) {
            auto& [l, r] = st[u].kids;
            if (st[u].flip) {
                swap(l, r);
                st[l].flip ^= 1;
                st[r].flip ^= 1;
                st[u].flip = 0;
                st[u].node.path_flip();
            }
            st[u].node.pushdown(st[l].node, st[r].node);
        }
    }

    void pushup(int u) {
        auto [l, r] = st[u].kids;
        st[u].node.pushup(st[l].node, st[r].node);
    }

    template <typename... Vs>
    int new_edge(Vs&&... args) {
        int e = freelist[F++];
        st[e].node = Node(forward<Vs>(args)...);
        return e;
    }

    void rem_edge(int e) {
        st[e] = LCTNode();
        freelist[--F] = e;
    }

    // ***** Interface
  public:
    template <typename... Vs>
    bool link(int u, int v, Vs&&... args) {
        reroot(u), access(v);
        if (st[u].parent)
            return false; // u and v are already connected

        int e = new_edge(forward<Vs>(args)...);
        st[e].parent = v, st[u].parent = e;
        pushup(u), pushup(e), pushup(v);
        return true;
    }

    bool cut(int u, int v) {
        reroot(u), access(v), fixup(u, v);
        if (!st[u].parent || u != st[v].kids[0])
            return false; // u and v are not connected at all

        int e = st[u].kids[1];
        if (!is_one(e))
            return false; // u and v are connected, but not directly by an edge

        st[u].parent = st[u].kids[1] = st[v].kids[0] = 0;
        rem_edge(e);
        pushup(u), pushup(v);
        return true;
    }

    void reroot(int u) {
        access(u);
        st[u].flip ^= 1, pushdown(u);
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
        return &st[u].node;
    }

    Node* access_path(int u, int v) {
        reroot(v), access(u);
        return &st[u].node;
    }

    Node* access_edge(int u, int v) {
        reroot(u), access(v), fixup(u, v);
        assert(is_one(st[u].kids[1]));
        return &st[st[u].kids[1]].node;
    }

  protected:
    bool is_root(int u) const {
        return st[st[u].parent].kids[0] != u && st[st[u].parent].kids[1] != u;
    }
    bool is_one(int u) const { return u && st[u].kids[0] == 0 && st[u].kids[1] == 0; }

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

    void fixup(int u, int p) {
        if (st[u].parent && st[u].parent != p) {
            pushdown(st[u].parent), pushdown(u), rotate(u);
        }
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
