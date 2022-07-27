#pragma once

#include "splay.hpp"

template <typename Node>
struct SegSplay : basic_splay<SegSplay<Node>> {
    int64_t key;
    Node self, sum;

    explicit SegSplay(int64_t key, Node data) : key(key), self(data), sum(sum) {}

    static auto get_key(const Splay* x) { return x->key; }
    static Node get_sum(const Splay* x) { return x ? x->sum : Node(); }

    void pushdown() {}

    void pushup() {
        if (this->kids[0] && this->kids[1]) {
            Node x;
            x.pushup(this->kids[0]->sum, self);
            sum.pushup(x, this->kids[1]->sum);
        } else if (this->kids[0]) {
            sum.pushup(this->kids[0]->sum, self);
        } else if (this->kids[1]) {
            sum.pushup(self, this->kids[1]->sum);
        } else {
            sum = self;
        }
    }

    static void update(Splay*& tree, int64_t key, Node data) {
        if (auto node = after(tree, key); node && node->key == key) {
            node->self = Node(data), tree = splay(node);
        } else {
            insert_before(tree, node, new Splay(key, data));
        }
    }
};

// Aggregate should be commutative and computable from only plain values
template <typename Node>
struct dynamic_2dsegtree {
    using Splay = SegSplay<Node>;
    int R, C;
    vector<Splay*> root;
    vector<array<int, 2>> kids;

    explicit dynamic_2dsegtree(int R, int C) : R(R), C(C) {}

    int build_sparse() {
        int u = root.size();
        root.push_back(nullptr);
        kids.push_back({0, 0});
        return u;
    }

    auto query(int u, int lx, int rx, int ly, int ry) {
        assert(0 <= lx && lx < rx && rx <= R && 0 <= ly && ly < ry && ry <= C);
        return query(u, 0, R, lx, rx, ly, ry);
    }

    int update(int u, int x, int y, Node value) {
        assert(0 <= x && x < R && 0 <= y && y < C);
        return update(u, 0, R, x, y, value);
    }

    auto query(int u, int l, int r, int lx, int rx, int ly, int ry) {
        if (u == 0) {
            return Node();
        } else if (lx <= l && r <= rx) {
            int64_t s = r - l, a = s * ly, b = s * ry;
            auto range = access_key_range(root[u], a, b);
            return range ? range->sum : Node();
        }
        int m = (l + r) / 2;
        auto [a, b] = kids[u];
        if (rx <= m) {
            return query(a, l, m, lx, rx, ly, ry);
        } else if (m <= lx) {
            return query(b, m, r, lx, rx, ly, ry);
        } else {
            Node ans;
            auto lhs = query(a, l, m, lx, rx, ly, ry);
            auto rhs = query(b, m, r, lx, rx, ly, ry);
            ans.pushup(lhs, rhs);
            return ans;
        }
    }

    int update(int u, int l, int r, int x, int y, Node value) {
        u = u == 0 ? build_sparse() : u;
        int64_t s = r - l, i = s * y + (x - l);
        Splay::update(root[u], i, value);
        if (l + 1 == r) {
            return u;
        }
        int m = (l + r) / 2;
        auto [a, b] = kids[u];
        if (x < m) {
            a = update(a, l, m, x, y, value);
        } else {
            b = update(b, m, r, x, y, value);
        }
        kids[u] = {a, b};
        return u;
    }
};
