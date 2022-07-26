#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename Node>
struct dynamic_segtree {
    vector<Node> node;
    vector<int> lazy, freelist;
    vector<array<int, 2>> kids;

    dynamic_segtree() = default;

    int num_nodes() const { return node.size(); }

    // Sparse: build an arbitrary sized tree where nodes just point back to themselves
    int build_sparse(Node init, bool rdonly) {
        if (rdonly) {
            int u = num_nodes();
            add_node(u, u, true, init);
            return u;
        } else {
            int u = num_nodes(), v = u + 1;
            add_node(v, v, false, init);
            add_node(v, v, true, init);
            return u;
        }
    }

    // Concatenation: # nodes must be a power of two and all have the same length
    int build_concat(const vector<int>& leaves, bool rdonly) {
        return build_concat_dfs(0, leaves.size(), leaves, rdonly);
    }

    // Levels: # levels lazy nodes are added
    int build_levels(int leaf, int levels, bool rdonly) {
        int u = leaf;
        while (levels--) {
            u = add_node(u, u, true, combine(node[u], node[u]));
        }
        lazy[u] = rdonly;
        return u;
    }

    // Array: For an array of size N, build a straightforward segtree with 2N-1 nodes
    template <typename T>
    int build_array(int N, const vector<T>& arr, bool rdonly, int s = 0) {
        return build_array_dfs(s, s + N, arr, rdonly);
    }

    template <typename... Us>
    void update_point(int root, int L, int R, int i, Us&&... update) {
        static thread_local vector<int> dfs;
        assert(L <= i && i < R);
        update_point_dfs(root, L, R, i, update);
    }

    template <typename... Us>
    void update_point_dfs(int u, int L, int R, int i, Us&&... update) {
        if (L + 1 == R) {
            apply(u, 1, update...);
            return;
        }
        pushdown(u, R - L);
        int M = (L + R) / 2;
        auto [a, b] = kids[u];
        if (i < M) {
            update_point_dfs(a, L, M, i, update...);
        } else {
            update_point_dfs(b, M, R, i, update...);
        }
        pushup(u);
    }

    template <typename... Us>
    void update_range(int root, int L, int R, int l, int r, Us&&... update) {
        assert(L <= l && l <= r && r <= R);
        if (l < r) {
            update_range_dfs(root, L, R, l, r, update...);
        }
    }

    auto query_point(int root, int L, int R, int i) {
        assert(L <= i && i < R);
        int u = root;
        while (L + 1 < R) {
            pushdown(u, R - L);
            int M = (L + R) / 2;
            if (i < M) {
                u = kids[u][0], R = M;
            } else {
                u = kids[u][1], L = M;
            }
        }
        return node[u];
    }

    auto query_range(int root, int L, int R, int l, int r) {
        assert(L <= l && l <= r && r <= R && L < R);
        return l == r ? Node() : query_range_dfs(root, L, R, l, r);
    }

    auto query_all(int root) { return node[root]; }

    template <typename Vis>
    auto visit_parents_up(int root, int L, int R, int i, Vis&& vis) {
        assert(L <= i && i < R);
        return visit_upwards(root, L, R, i, vis);
    }

    template <typename Vis>
    auto visit_parents_down(int root, int L, int R, int i, Vis&& vis) {
        return visit_downwards(root, L, R, i, vis);
    }

    template <typename Vis>
    void visit_range_l_to_r(int root, int L, int R, int l, int r, Vis&& vis) {
        assert(L <= l && l <= r && r <= R);
        if (l < r) {
            visit_range_l_to_r_dfs(root, L, R, l, r, vis);
        }
    }

    template <typename Vis>
    void visit_range_r_to_l(int root, int L, int R, int l, int r, Vis&& vis) {
        assert(L <= l && l <= r && r <= R);
        if (l < r) {
            visit_range_r_to_l_dfs(root, L, R, l, r, vis);
        }
    }

    // Binary search with Bs(prefix) on the range [0,N) for the False/True split
    // Aggregates the entire segment tree prefix.  F F F F >T< T T T [N=T)
    // Returns {index i of first truth, prefix aggregate [0,u)}
    template <typename Bs>
    auto prefix_binary_search(int root, int L, int R, Bs&& bs) {
        assert(L < R);
        int u = root;
        Node prefix = Node();
        while (L + 1 < R) {
            pushdown(u, R - L);
            int M = (L + R) / 2;
            auto [a, b] = kids[u];
            Node v = combine(prefix, node[a]);
            if (bs(v)) {
                u = a, R = M;
            } else {
                prefix = move(v);
                u = b, L = M;
            }
        }
        Node v = combine(prefix, node[u]);
        return bs(v) ? make_pair(L, move(prefix)) : make_pair(R, move(v));
    }

    // Binary search with Bs(suffix) on the range [0,N) for the False/True split
    // Aggregates the segment tree suffix. F F F F >T< T T T [N=T)
    // Returns {index i of first truth, suffix aggregate [u,N)}
    template <typename Bs>
    auto suffix_binary_search(int root, int L, int R, Bs&& bs) {
        assert(L < R);
        int u = root;
        Node suffix = Node();
        while (L + 1 < R) {
            pushdown(u, R - L);
            int M = (L + R) / 2;
            auto [a, b] = kids[u];
            Node v = combine(node[b], suffix);
            if (bs(v)) {
                suffix = move(v);
                u = a, R = M;
            } else {
                u = b, L = M;
            }
        }
        Node v = combine(node[u], suffix);
        return bs(v) ? make_pair(L, move(v)) : make_pair(R, move(suffix));
    }

    // Binary search with Bs(prefix) on the range [l,r) for the False/True split
    // Aggregate only values within this range. F F F F >T< T T [r=T)
    // Returns {index i of first truth, prefix aggregate [l,i)}
    template <typename Bs>
    auto prefix_range_search(int root, int L, int R, int l, int r, Bs&& bs) {
        assert(L <= l && l <= r && r <= R);
        return l == r ? make_pair(r, Node())
                      : run_prefix_search(root, L, R, l, r, Node(), bs);
    }

    // Binary search with Bs(suffix) on the range [l,r) for the False/True split
    // Aggregate only values within this range. F F F F >T< T T [r=T)
    // Returns {index i of first truth, suffix aggregate [i,r)}
    template <typename Bs>
    auto suffix_range_search(int root, int L, int R, int l, int r, Bs&& bs) {
        assert(L <= l && l <= r && r <= R);
        return l == r ? make_pair(r, Node())
                      : run_suffix_search(root, L, R, l, r, Node(), bs);
    }

    int meld(int u, int v, int L, int R, int zero) {
        static_assert(!Node::LAZY);
        assert(L < R);
        return run_meld(u, v, L, R, zero);
    }

  private:
    static Node combine(const Node& x, const Node& y) {
        Node ans;
        ans.pushup(x, y);
        return ans;
    }

    inline int make_node() {
        if (freelist.size()) {
            int v = freelist.back();
            freelist.pop_back();
            node[v] = Node();
            return v;
        } else {
            int v = node.size();
            node.push_back(Node());
            lazy.push_back(false);
            kids.push_back({0, 0});
            return v;
        }
    }

    inline int add_node(int l, int r, int8_t rdonly, Node v) {
        if (freelist.size()) {
            int u = freelist.back();
            freelist.pop_back();
            node[u] = move(v);
            lazy[u] = rdonly;
            kids[u] = {l, r};
            return u;
        } else {
            int u = node.size();
            node.push_back(move(v));
            lazy.push_back(rdonly);
            kids.push_back({l, r});
            return u;
        }
    }

    inline int maybe_clone_node(int u) {
        return lazy[u] ? add_node(kids[u][0], kids[u][1], false, node[u]) : u;
    }

    template <typename... Us>
    inline void apply(int u, int S, Us&&... update) {
        if constexpr (Node::RANGES) {
            node[u].apply(update..., S);
        } else {
            node[u].apply(update...);
        }
    }

    inline void pushup(int u) { node[u].pushup(node[kids[u][0]], node[kids[u][1]]); }

    void pushdown(int u, int s) {
        int a = kids[u][0] = maybe_clone_node(kids[u][0]);
        int b = kids[u][1] = maybe_clone_node(kids[u][1]);
        if constexpr (!Node::LAZY) {
            return;
        } else if constexpr (Node::RANGES) {
            node[u].pushdown(node[a], node[b], s / 2, (s + 1) / 2);
        } else {
            node[u].pushdown(node[a], node[b]);
        }
    }

    template <typename T>
    int build_array_dfs(int L, int R, const vector<T>& arr, bool rdonly) {
        if (L + 1 == R) {
            return add_node(-1, -1, rdonly, Node(arr[L]));
        } else {
            int M = (L + R) / 2;
            int a = build_array_dfs(L, M, arr, rdonly);
            int b = build_array_dfs(M, R, arr, rdonly);
            return add_node(a, b, rdonly, combine(node[a], node[b]));
        }
    }

    int build_concat_dfs(int L, int R, const vector<int>& leaves, bool rdonly) {
        if (L + 1 == R) {
            return leaves[L];
        } else {
            int M = (L + R) / 2;
            int a = build_concat_dfs(L, M, leaves, rdonly);
            int b = build_concat_dfs(M, R, leaves, rdonly);
            return add_node(a, b, rdonly, combine(node[a], node[b]));
        }
    }

    template <typename... Us>
    void update_range_dfs(int u, int L, int R, int ql, int qr, Us&&... update) {
        if (ql <= L && R <= qr) {
            apply(u, R - L, update...);
            return;
        }
        pushdown(u, R - L);
        int M = (L + R) / 2;
        auto [a, b] = kids[u];
        if (qr <= M) {
            update_range_dfs(a, L, M, ql, qr, update...);
        } else if (M <= ql) {
            update_range_dfs(b, M, R, ql, qr, update...);
        } else {
            update_range_dfs(a, L, M, ql, M, update...);
            update_range_dfs(b, M, R, M, qr, update...);
        }
        pushup(u);
    }

    auto query_range_dfs(int u, int L, int R, int ql, int qr) {
        if (ql <= L && R <= qr) {
            return node[u];
        }
        pushdown(u, R - L);
        int M = (L + R) / 2;
        auto [a, b] = kids[u];
        if (qr <= M) {
            return query_range_dfs(a, L, M, ql, qr);
        } else if (M <= ql) {
            return query_range_dfs(b, M, R, ql, qr);
        } else {
            return combine(query_range_dfs(a, L, M, ql, M),
                           query_range_dfs(b, M, R, M, qr));
        }
    }

    template <typename Vis>
    void visit_upwards(int u, int L, int R, int q, Vis&& vis) {
        if (L + 1 < R) {
            pushdown(u, R - L);
            int M = (L + R) / 2;
            auto& [a, b] = kids[u];
            if (q < M) {
                a = maybe_clone_node(a);
                visit_upwards(a, L, M, q, vis);
            } else {
                b = maybe_clone_node(b);
                visit_upwards(b, M, R, q, vis);
            }
            pushup(u);
            vis(node[u], L, R);
        } else {
            vis(node[u], L, R);
        }
    }

    template <typename Vis>
    void visit_downwards(int u, int L, int R, int q, Vis&& vis) {
        if (L + 1 < R) {
            pushdown(u, R - L);
            vis(node[u], L, R);
            int M = (L + R) / 2;
            auto& [a, b] = kids[u];
            if (q < M) {
                a = maybe_clone_node(a);
                visit_downwards(a, L, M, q, vis);
            } else {
                b = maybe_clone_node(b);
                visit_downwards(b, M, R, q, vis);
            }
            pushup(u);
        } else {
            vis(node[u], L, R);
        }
    }

    template <typename Vis>
    void visit_range_l_to_r_dfs(int u, int L, int R, int ql, int qr, Vis&& vis) {
        if (ql <= L && R <= qr) {
            vis(node[u], L, R);
            return;
        }
        pushdown(u, R - L);
        int M = (L + R) / 2;
        auto [a, b] = kids[u];
        if (qr <= M) {
            visit_range_l_to_r_dfs(a, L, M, ql, qr, vis);
        } else if (M <= ql) {
            visit_range_l_to_r_dfs(b, M, R, ql, qr, vis);
        } else {
            visit_range_l_to_r_dfs(a, L, M, ql, M, vis);
            visit_range_l_to_r_dfs(b, M, R, M, qr, vis);
        }
        pushup(u);
    }

    template <typename Vis>
    void visit_range_r_to_l_dfs(int u, int L, int R, int ql, int qr, Vis&& vis) {
        if (ql <= L && R <= qr) {
            vis(node[u]);
            return;
        }
        pushdown(u, R - L);
        int M = (L + R) / 2;
        auto [a, b] = kids[u];
        if (qr <= M) {
            visit_range_r_to_l_dfs(a, L, M, ql, qr, vis);
        } else if (M <= ql) {
            visit_range_r_to_l_dfs(b, M, R, ql, qr, vis);
        } else {
            visit_range_r_to_l_dfs(b, M, R, M, qr, vis);
            visit_range_r_to_l_dfs(a, L, M, ql, M, vis);
        }
        pushup(u);
    }

    template <typename Bs>
    auto run_prefix_search(int u, int L, int R, int ql, int qr, Node prefix, Bs&& bs) {
        if (ql <= L && R <= qr) {
            Node extra = combine(prefix, node[u]);
            if (bs(extra)) {
                while (L + 1 < R) {
                    pushdown(u, R - L);
                    int M = (L + R) / 2;
                    auto [a, b] = kids[u];
                    Node v = combine(prefix, node[a]);
                    if (bs(v)) {
                        u = a, R = M;
                    } else {
                        prefix = move(v);
                        u = b, L = M;
                    }
                }
                Node v = combine(prefix, node[u]);
                return bs(v) ? make_pair(L, move(prefix)) : make_pair(R, move(v));
            } else {
                return make_pair(R, move(extra));
            }
        }
        pushdown(u, R - L);
        int x, M = (L + R) / 2;
        auto [a, b] = kids[u];
        if (qr <= M) {
            return run_prefix_search(a, L, M, ql, qr, move(prefix), bs);
        } else if (M <= ql) {
            return run_prefix_search(b, M, R, ql, qr, move(prefix), bs);
        }
        tie(x, prefix) = run_prefix_search(a, L, M, ql, M, move(prefix), bs);
        if (x < M) {
            return make_pair(x, move(prefix));
        } else {
            return run_prefix_search(b, M, R, M, qr, move(prefix), bs);
        }
    }

    template <typename Bs>
    auto run_suffix_search(int u, int L, int R, int ql, int qr, Node suffix, Bs&& bs) {
        if (ql <= L && R <= qr) {
            Node extra = combine(node[u], suffix);
            if (!bs(extra)) {
                while (L + 1 < R) {
                    pushdown(u, R - L);
                    int M = (L + R) / 2;
                    auto [a, b] = kids[u];
                    Node v = combine(node[b], suffix);
                    if (bs(v)) {
                        suffix = move(v);
                        u = a, R = M;
                    } else {
                        u = b, L = M;
                    }
                }
                Node v = combine(node[u], suffix);
                return bs(v) ? make_pair(L, move(v)) : make_pair(R, move(suffix));
            } else {
                return make_pair(L, move(extra));
            }
        }
        pushdown(u, R - L);
        int x, M = (L + R) / 2;
        auto [a, b] = kids[u];
        if (qr <= M) {
            return run_suffix_search(a, L, M, ql, qr, move(suffix), bs);
        } else if (M <= ql) {
            return run_suffix_search(b, M, R, ql, qr, move(suffix), bs);
        }
        tie(x, suffix) = run_suffix_search(b, M, R, M, qr, move(suffix), bs);
        if (x > M) {
            return make_pair(x, move(suffix));
        } else {
            return run_suffix_search(a, L, M, ql, M, move(suffix), bs);
        }
    }

    int run_meld(int u, int v, int L, int R, int zero) {
        if (u == zero || v == zero) {
            return u ^ v ^ zero;
        } else if (L + 1 == R) {
            node[u].meld(node[v]);
            freelist.push_back(v);
            return u;
        } else {
            pushdown(u, R - L);
            pushdown(v, R - L);
            int M = (L + R) / 2;
            int a = run_meld(kids[u][0], kids[v][0], L, M, kids[zero][0]);
            int b = run_meld(kids[u][1], kids[v][1], M, R, kids[zero][1]);
            kids[u] = {a, b};
            pushup(u);
            freelist.push_back(v);
            return u;
        }
    }
};

struct Segnode {
    static constexpr bool LAZY = true, RANGES = true;
    long value = 0, lazy = 0;

    Segnode(long value = 0) : value(value) {}

    void pushup(const Segnode& lhs, const Segnode& rhs) {
        assert(lazy == 0);
        value = lhs.value + rhs.value;
    }

    void pushdown(Segnode& lhs, Segnode& rhs, int a, int b) {
        if (lazy) {
            lhs.apply(lazy, a);
            rhs.apply(lazy, b);
            lazy = 0;
        }
    }

    void apply(long add, int s) {
        value += s * add;
        lazy += add;
    }
};
