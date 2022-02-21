#pragma once

#include <bits/stdc++.h>
using namespace std;

template <typename T>
struct segnode {
    static constexpr bool LAZY = true;
    using ipos = int; // or long

    // maintain 'self' value, 'subt' aggregate and 'lazy' subtree propagation tags
    // define 5 fns: static mult(), update_self(), update_subt(), pushup(), pushdown()
    // IMPORTANT: update_subt should do nothing if subt_size=0.
    // update_self does not need to refresh subt aggregate and should not store lazy tag
    // update_subt should update subt aggregate and store lazy tag

    T self = 0;
    T subt = 0;
    T lazy = 0;

    auto range_aggregate() const { return subt; }
    auto point_value() const { return self; }

    static auto mult(ipos unit_size, const segnode& node) {
        segnode ans;
        ans.self = node.self;
        ans.subt = node.self * unit_size; // required
        return ans;
    }

    void update_self(T update) { self += update; }

    void update_subt(T update, ipos subt_size) {
        if (subt_size) { // required
            self += update;
            subt += update * subt_size;
            lazy += update;
        }
    }

    void pushup(const segnode& lhs, const segnode& rhs, ipos self_size) {
        subt = self * self_size + lhs.subt + rhs.subt;
    }

    void pushdown(segnode& lhs, segnode& rhs, ipos L, ipos R) {
        if (lazy != 0) {
            lhs.update_subt(lazy, L);
            rhs.update_subt(lazy, R);
            lazy = 0;
        }
    }
};

/**
 * A (sparse) segment tree built over a splay tree.
 *
 * Complexity: O(U) memory and O(log U) time per query/update, where U = min(N,#updates)
 */
template <typename Node>
struct splay_segtree {
    using ipos = typename Node::ipos;

    struct Splay {
        int parent = 0;
        array<int, 2> kids = {};
        array<ipos, 2> unit = {};
        array<ipos, 2> subt = {};
        Node node;
    };

    explicit splay_segtree(ipos L, ipos R, Node init = Node()) { assign(L, R, init); }

    void assign(ipos L, ipos R, Node init = Node()) {
        root = 1;
        st.assign(2, Splay());
        st[1] = {0, {0, 0}, {L, R}, {L, R}, Node::mult(R - L, init)};
    }

    int num_nodes() const { return st.size() - 1; }

  private: // ***** Segtree operations
    int root;
    vector<Splay> st;

    inline ipos unitlen(int u) const { return st[u].unit[1] - st[u].unit[0]; }
    inline ipos subtlen(int u) const { return st[u].subt[1] - st[u].subt[0]; }

    auto combine(const Node& a, const Node& b) {
        Node c;
        c.pushup(a, b, 0);
        return c;
    }

    void pushup(int u) {
        auto [l, r] = st[u].kids;
        st[u].node.pushup(st[l].node, st[r].node, unitlen(u));
        st[u].subt[0] = l ? st[l].subt[0] : st[u].unit[0];
        st[u].subt[1] = r ? st[r].subt[1] : st[u].unit[1];
    }

    void pushdown([[maybe_unused]] int u) {
        if constexpr (Node::LAZY) {
            auto [l, r] = st[u].kids;
            st[u].node.pushdown(st[l].node, st[r].node, subtlen(l), subtlen(r));
        }
    }

    int clone_node(int parent, ipos L, ipos R, int u) {
        st.push_back({parent, {0, 0}, {L, R}, {L, R}, Node::mult(R - L, st[u].node)});
        return num_nodes();
    }

    template <int8_t side>
    int limit_node(int u) {
        pushdown(u);
        while (st[u].kids[side]) {
            u = st[u].kids[side];
            pushdown(u);
        }
        return u;
    }

    // find the representative of segtree position i
    int find_node(ipos i) {
        int u = root;
        while (true) {
            pushdown(u);
            auto [L, R] = st[u].unit;
            if (i < L) {
                u = st[u].kids[0];
            } else if (R <= i) {
                u = st[u].kids[1];
            } else {
                return u;
            }
        }
    }

    // split the node u=[L,R) into [L,M)/[M,R), create v=[L,M) before u and return v
    auto split_before(int u, ipos M) {
        auto [L, R] = st[u].unit;
        assert(L < M && M < R);
        if (st[u].kids[0]) {
            int w = limit_node<1>(st[u].kids[0]);
            st[w].kids[1] = clone_node(w, L, M, u);
        } else {
            st[u].kids[0] = clone_node(u, L, M, u);
        }
        st[u].unit = {M, R};
        return num_nodes();
    }

    // split the node u=[L,R) into [L,M)/[M,R), create v=[M,R) after u and return v
    auto split_after(int u, ipos M) {
        auto [L, R] = st[u].unit;
        assert(L < M && M < R);
        if (st[u].kids[1]) {
            int w = limit_node<0>(st[u].kids[1]);
            st[w].kids[0] = clone_node(w, M, R, u);
        } else {
            st[u].kids[1] = clone_node(u, M, R, u);
        }
        st[u].unit = {L, M};
        return num_nodes();
    }

  private: // ***** Splay tree operations
    void adopt(int parent, int kid, int8_t side) {
        if (side >= 0)
            st[parent].kids[side] = kid;
        if (kid)
            st[kid].parent = parent;
    }

    void rotate(int u) {
        int p = st[u].parent, g = st[p].parent;
        int8_t my_side = u == st[p].kids[1];
        int8_t p_side = g ? p == st[g].kids[1] : -1;
        adopt(p, st[u].kids[!my_side], my_side);
        adopt(g, u, p_side);
        adopt(u, p, !my_side);
        pushup(p);
    }

    int splay(int u) {
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
        return root = u;
    }

    int maybe_splay(int u) { return u != 0 ? splay(u) : 0; }

    int range_splay(int u, int v) {
        splay(v), splay(u);
        if (st[v].parent != u) {
            rotate(v);
        }
        int s = st[v].kids[0];
        if (s != 0) {
            pushdown(s), pushup(s);
        }
        return s;
    }

    template <typename Fn>
    void visit_inorder(int u, Fn&& fn) {
        pushdown(u);
        if (st[u].kids[0]) {
            visit_inorder(st[u].kids[0], fn);
        }
        fn(st[u].node, st[u].unit);
        if (st[u].kids[1]) {
            visit_inorder(st[u].kids[1], fn);
        }
    }

  public:
    template <typename Fn>
    void visit_all(Fn&& fn) {
        visit_inorder(root, fn);
    }

    Node query_point(ipos i) {
        int u = find_node(i);
        splay(u);
        return st[u].node;
    }

    template <typename Update>
    void update_point(ipos i, Update&& update) {
        int u = find_node(i), l = 0, r = 0;
        auto [L, R] = st[u].unit;
        if (L != i) {
            l = split_before(u, i);
        }
        if (R != i + 1) {
            r = split_after(u, i + 1);
        }
        maybe_splay(l);
        maybe_splay(r);
        st[u].node.update_self(update);
        splay(u);
    }

    Node query_range(ipos L, ipos R) {
        int v = find_node(R - 1), u = find_node(L);
        if (u == v) {
            splay(u);
            return Node::mult(R - L, st[u].node);
        }
        int s = range_splay(u, v);
        auto A = st[u].unit[1], B = st[v].unit[0];
        Node x = Node::mult(A - L, st[u].node);
        Node y = st[s].node;
        Node z = Node::mult(R - B, st[v].node);
        // merging them this way ensures we don't need commutativity
        z.pushup(y, Node(), R - B);
        x.pushup(Node(), z, A - L);
        maybe_splay(s);
        return x;
    }

    template <typename Update>
    void update_range(ipos L, ipos R, Update&& update) {
        static_assert(Node::LAZY);
        int v = find_node(R - 1), right = 0;
        if (auto vR = st[v].unit[1]; R != vR) {
            right = split_after(v, R);
        }
        int u = find_node(L), left = 0;
        if (auto uL = st[u].unit[0]; L != uL) {
            left = split_before(u, L);
        }
        if (u == v) {
            maybe_splay(left);
            maybe_splay(right);
            st[u].node.update_self(update);
            splay(u);
            return;
        }
        if (left == 0) {
            st[u].node.update_self(update), left = u;
        }
        if (right == 0) {
            st[v].node.update_self(update), right = v;
        }
        if (int s = range_splay(left, right); s != 0) {
            st[s].node.update_subt(update, subtlen(s));
            splay(s);
        }
    }

    auto query_all() {
        pushdown(root);
        return st[root].node;
    }

    template <typename Update>
    void update_all(Update&& update) {
        static_assert(Node::LAZY);
        st[root].node.update_subt(update, subtlen(root));
    }

    template <typename Fn> // [L=? F F F F {T} T T T T T) R=?
    ipos first_true(ipos L, ipos R, Fn&& fn) {
        int u = splay(find_node(L));
        ipos B = min(st[u].unit[1], R);
        Node prev = Node::mult(B - L, st[u].node);
        if (fn(prev)) {
            ipos A = L;
            while (A + 1 < B) {
                ipos M = A + (B - A) / 2;
                Node v = Node::mult(M - L, st[u].node);
                (fn(v) ? B : A) = M;
            }
            return A;
        } else if (B == R) {
            return R;
        }

        u = st[u].kids[1];
        while (u && st[u].subt[0] <= R) {
            while (u && st[u].unit[0] >= R) {
                pushdown(u);
                u = st[u].kids[0];
            }
            pushdown(u);
            Node cur = prev; // [L,st[u].subt[0])
            if (int l = st[u].kids[0]; l) {
                cur = combine(prev, st[l].node); // [L,st[u].unit[0])
                if (fn(cur)) {
                    R = st[u].unit[0], u = l;
                    continue;
                }
            }
            prev = move(cur);
            B = min(st[u].unit[1], R);
            cur = combine(prev, Node::mult(B - st[u].unit[0], st[u].node));
            if (fn(cur)) {
                ipos A = L = st[u].unit[0];
                while (A + 1 < B) {
                    ipos M = A + (B - A) / 2;
                    Node v = combine(prev, Node::mult(M - L, st[u].node));
                    (fn(v) ? B : A) = M;
                }
                splay(u); // principled splay
                return A;
            } else if (R == B) {
                return R;
            } else {
                prev = move(cur);
                u = st[u].kids[1];
            }
        }
        return R;
    }

    template <typename Fn>
    ipos first_true(Fn&& fn) {
        return first_true(st[root].subt[0], st[root].subt[1], forward<Fn>(fn));
    }
};
