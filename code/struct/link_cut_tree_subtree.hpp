#pragma once

#include <bits/stdc++.h>
using namespace std;

struct lct_node_subtree_empty {
    static constexpr bool LAZY = true, VIRTUAL_PUSHUP = false;
    void flip_path() {}
    void pushdown(lct_node_subtree_empty&, lct_node_subtree_empty&) {}
    void pushup(const lct_node_subtree_empty&, const lct_node_subtree_empty&) {}
    void add_virtual_subtree(lct_node_subtree_empty&) {}
    void rem_virtual_subtree(lct_node_subtree_empty&) {}
};

/**
 * Unrooted link cut tree: path/subtree queries + path/point updates (nodes 1-indexed)
 */
template <typename Node>
struct link_cut_tree_subtree {
    struct LCTNode {
        int parent = 0, kids[2] = {};
        int8_t flip = 0; // splay tree is flipped due to reroot
        Node data;
    };

    vector<LCTNode> st;

    explicit link_cut_tree_subtree(int N = 0) : st(N + 1) {}

    template <typename T>
    link_cut_tree_subtree(int N, const vector<T>& arr) : st(N + 1) {
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

    void add_virtual_subtree(int u, int c) {
        if (c == 0) {
            return;
        } else if constexpr (Node::VIRTUAL_PUSHUP) {
            auto [l, r] = st[u].kids;
            st[u].data.add_virtual_subtree(st[l].data, st[r].data, st[c].data);
        } else {
            st[u].data.add_virtual_subtree(st[c].data);
        }
    }

    void rem_virtual_subtree(int u, int c) {
        if (c == 0) {
            return;
        } else if constexpr (Node::VIRTUAL_PUSHUP) {
            auto [l, r] = st[u].kids;
            st[u].data.rem_virtual_subtree(st[l].data, st[r].data, st[c].data);
        } else {
            st[u].data.rem_virtual_subtree(st[c].data);
        }
    }

    // ***** Interface
  public:
    bool link(int u, int v) {
        reroot(u), access(v);
        if (st[u].parent)
            return false;
        st[u].parent = v;
        add_virtual_subtree(v, u);
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

    Node* access_subtree(int u, int v) {
        reroot(v), access(u);
        return &st[u].data;
    }

    Node* access_tree(int u) {
        reroot(u), access(u);
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
            add_virtual_subtree(v, st[v].kids[1]);
            rem_virtual_subtree(v, last);
            st[v].kids[1] = last;
            pushup(v);
            last = v, v = st[v].parent;
        } while (v);
        splay(u);
        assert(!st[u].kids[1] && !st[u].flip);
        return last;
    }
};

/**
 * Maintain sum and size of subtrees and paths
 */
struct lct_node_complete_sum {
    static constexpr bool LAZY = true, VIRTUAL_PUSHUP = false;
    int path_size = 0;
    int subt_size = 0; // size of splay tree below u
    int virt_size = 0; // size of subtree below u
    int64_t self = 0;  // this data's value
    int64_t path = 0;
    int64_t lazy = 0;
    int64_t subt = 0; // subtree aggregate ~= aggregate of splay + virtuals
    int64_t virt = 0; // virtual aggregate ~= aggregate of virtuals
    // subtree query is self + virt / subtree size query is 1 + virt_size

    lct_node_complete_sum() = default;
    lct_node_complete_sum(int64_t v)
        : path_size(1), subt_size(1), self(v), path(v), subt(v) {}

    int64_t subtree_sum() const { return self + virt; }
    int64_t path_sum() const { return path; }
    int subtree_size() const { return 1 + virt_size; }
    int path_length() const { return path_size; }

    void add_node(int64_t value) { self += value; }

    void add_path(int64_t plus) {
        if (path_size) {
            lazy += plus;
            self += plus;
            path += plus * path_size;
            subt += plus * path_size;
        }
    }

    void flip_path() {}

    void pushdown(lct_node_complete_sum& lhs, lct_node_complete_sum& rhs) {
        if (lazy) {
            lhs.add_path(lazy);
            rhs.add_path(lazy);
            lazy = 0;
        }
    }

    void pushup(const lct_node_complete_sum& lhs, const lct_node_complete_sum& rhs) {
        path_size = 1 + lhs.path_size + rhs.path_size;
        subt_size = 1 + lhs.subt_size + rhs.subt_size + virt_size;
        path = self + lhs.path + rhs.path;
        subt = self + lhs.subt + rhs.subt + virt;
    }

    void add_virtual_subtree(lct_node_complete_sum& child) {
        virt += child.subt;
        virt_size += child.subt_size;
    }

    void rem_virtual_subtree(lct_node_complete_sum& child) {
        virt -= child.subt;
        virt_size -= child.subt_size;
    }
};

/**
 * Maintain sum and size of subtrees
 */
struct lct_node_subtree_sum {
    static constexpr bool LAZY = true, VIRTUAL_PUSHUP = false;
    int subt_size = 0; // size of splay tree below u
    int virt_size = 0; // size of subtree below u
    int64_t self = 0;  // this data's value
    int64_t subt = 0;  // subtree aggregate ~= aggregate of splay + virtuals
    int64_t virt = 0;  // virtual aggregate ~= aggregate of virtuals
    // subtree query is self + virt / subtree size query is 1 + virt_size

    lct_node_subtree_sum() = default;
    lct_node_subtree_sum(int64_t v) : subt_size(1), self(v), subt(v) {}

    int64_t subtree_sum() const { return self + virt; }
    int subtree_size() const { return 1 + virt_size; }

    void add_node(int64_t value) { self += value, subt += value; }

    void flip_path() {}

    void pushdown(lct_node_subtree_sum&, lct_node_subtree_sum&) {}

    void pushup(const lct_node_subtree_sum& lhs, const lct_node_subtree_sum& rhs) {
        subt_size = 1 + lhs.subt_size + rhs.subt_size + virt_size;
        subt = self + lhs.subt + rhs.subt + virt;
    }

    void add_virtual_subtree(lct_node_subtree_sum& child) {
        virt += child.subt;
        virt_size += child.subt_size;
    }

    void rem_virtual_subtree(lct_node_subtree_sum& child) {
        virt -= child.subt;
        virt_size -= child.subt_size;
    }
};
