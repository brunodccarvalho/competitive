#pragma once

#include "splay.hpp"

// Splay tree that represents the branches of a bounded convex piecewise linear function
// with integer slopes, dangling on an implicit vertex. Supports pointwise addition and
// minplus convolution (small to large), inversion and evaluation in O(log n)
struct Branch : basic_splay<Branch> {
    using V = int64_t;
    static constexpr bool SAFE = true; // set to true to allow merging
    int size = 1, flip = 0;
    V slope = 0, width = 0, sum = 0, height = 0, lazy = 0;

    explicit Branch(V slope, V len)
        : slope(slope), width(len), sum(len), height(slope * len) {}

    static int get_size(const Branch* x) { return x ? x->size : 0; }
    static V get_key(const Branch* x) { return x->slope; }
    static V get_sum(const Branch* x) { return x ? x->sum : 0; }
    static V get_height(const Branch* x) { return x ? x->height : 0; }

    inline V val() const { return width * slope; }
    void adjust_width(V add) { width += add, sum += add, height += add * slope; }
    void update_range(V add) { slope += add, lazy += add, height += add * sum; }
    void invert_range() { flip ^= 1, slope = -slope, height = -height, lazy = -lazy; }

    void pushdown() {
        if (flip) {
            if (kids[0])
                kids[0]->invert_range();
            if (kids[1])
                kids[1]->invert_range();
            swap(kids[0], kids[1]);
            flip = 0;
        }
        if (lazy) {
            if (kids[0])
                kids[0]->update_range(lazy);
            if (kids[1])
                kids[1]->update_range(lazy);
            lazy = 0;
        }
    }

    void pushup() {
        sum = width + get_sum(kids[0]) + get_sum(kids[1]);
        size = 1 + get_size(kids[0]) + get_size(kids[1]);
        height = val() + get_height(kids[0]) + get_height(kids[1]);
    }

    static void invert(Branch*& tree) {
        if (tree) {
            tree->invert_range();
        }
    }

    static void insert_segment(Branch*& tree, V slope, V width) {
        if (width <= 0) {
            return;
        } else if (auto pos = after(tree, slope); SAFE && pos && pos->slope <= slope) {
            pos->adjust_width(width), tree = splay(pos);
        } else {
            insert_before(tree, pos, new Branch(slope, width));
        }
    }

    // Find the node responsible for x=rank. If this lies at the beginning of a node
    // x, it returns x, otherwise it cuts x and returns its successor node
    static auto find_by_rank(Branch*& tree, V rank) -> Branch* {
        if (!tree || get_sum(tree) <= rank) {
            return nullptr;
        } else if (rank <= 0) {
            return front(tree);
        }
        Branch *node = tree, *after = nullptr;
        while (node) {
            node->pushdown();
            if (rank < get_sum(node->kids[0])) {
                after = node;
                node = node->kids[0];
            } else if (rank -= get_sum(node->kids[0]); rank < node->width) {
                if (rank <= 0) {
                    return tree = splay(node);
                } else {
                    V rest = node->width - rank;
                    node->adjust_width(rank - node->width);
                    tree = splay(node);
                    return insert_after(tree, node, new Branch(node->slope, rest));
                }
            } else {
                rank -= node->width;
                node = node->kids[1];
            }
        }
        if (after) {
            auto b = new Branch(after->slope, 0);
            return insert_before(tree, after, b);
        } else {
            auto b = new Branch(back(tree)->slope, 0);
            return push_back(tree, b);
        }
    }

    // Find the earliest x for which slope after x is >= to given slope
    static V slope_rank(Branch*& tree, V slope) {
        auto node = after(tree, slope);
        return node ? get_sum(node->kids[0]) : get_sum(tree);
    }

    // Find the range [L,R] of ranks where function attains values <=ceiling.
    // If no range satisfies this it returns {-1,-1}. O(log n)
    static auto ceiling_range(Branch*& tree, V ceiling) {
        auto [left, zero, right] = split_key_range(tree, 0, 1);
        auto bottom = get_height(left);
        pair<V, V> ans = {-1, -1};

        if (ceiling < bottom) {
            tree = join(left, zero, right);
            return ans;
        }

        ans = {get_sum(left), get_sum(left) + get_sum(zero)};
        V prefix = bottom - ceiling, suffix = ceiling - bottom;
        Branch *node, *previous;

        node = previous = left;
        while (node) {
            previous = node, node->pushdown();
            if (prefix > get_height(node->kids[1])) {
                node = node->kids[1];
            } else if (prefix -= get_height(node->kids[1]); prefix > node->val()) {
                ans.first -= get_sum(node->kids[1]) + (prefix / node->slope);
                break;
            } else {
                prefix -= node->val();
                ans.first -= get_sum(node->kids[1]) + node->width;
                node = node->kids[0];
            }
        }
        left = splay(previous);

        node = previous = right;
        while (node) {
            previous = node, node->pushdown();
            if (suffix < get_height(node->kids[0])) {
                node = node->kids[0];
            } else if (suffix -= get_height(node->kids[0]); suffix < node->val()) {
                ans.second += get_sum(node->kids[0]) + (suffix / node->slope);
                break;
            } else {
                suffix -= node->val();
                ans.second += get_sum(node->kids[0]) + node->width;
                node = node->kids[1];
            }
        }
        right = splay(previous);

        tree = join(left, zero, right);
        return ans;
    }

    // Split the tree into two branches [0,rank) and [rank,...)
    static auto split_rank_range(Branch*& tree, V rank) {
        auto A = find_by_rank(tree, rank);
        return split_before(tree, A);
    }

    // Query height at given rank. O(log n)
    static V query(Branch*& tree, V rank) {
        if (rank <= 0) {
            return 0;
        } else if (get_sum(tree) <= rank) {
            return get_height(tree);
        }
        V prefix = 0;
        Branch* previous = tree;
        Branch* node = tree;
        while (node) {
            previous = node;
            node->pushdown();
            if (rank < get_sum(node->kids[0])) {
                node = node->kids[0];
            } else if (rank -= get_sum(node->kids[0]); rank < node->width) {
                prefix += get_height(node->kids[0]) + node->slope * rank;
                break;
            } else {
                prefix += get_height(node->kids[0]) + node->slope * node->width;
                rank -= node->width;
                node = node->kids[1];
            }
        }
        tree = splay(previous);
        return prefix;
    }

    // Minplus convolution of two branches, the caller tracks the vertex shift.
    static void minplus(Branch*& a, Branch*& b) { a = meld(a, b), b = nullptr; }

    // Pointwise addition of two branches, the caller tracks the vertex shift.
    // Small to large. O(min(a,b) log max(a,b)). Restricts to common domain.
    static void pointwise(Branch*& a, Branch*& b, V av, V bv) {
        V A = get_sum(a), ar = av + A;
        V B = get_sum(b), br = bv + B;
        trim_front(a, max<V>(0, bv - av));
        trim_front(b, max<V>(0, av - bv));
        trim_back(a, max<V>(0, ar - br));
        trim_back(b, max<V>(0, br - ar));
        if (get_size(a) < get_size(b)) {
            swap(a, b);
        }
        vector<Branch*> from = unstitch(b);
        vector<Branch*> into = {a};
        int S = from.size();
        for (int i = 0; i < S; i++) {
            auto [u, v] = split_rank_range(into.back(), from[i]->width);
            into.pop_back();
            into.push_back(u);
            into.push_back(v);
        }
        for (int i = 0; i < S; i++) {
            if (into[i]) {
                into[i]->update_range(from[i]->slope);
            }
            delete from[i];
        }
        for (int i = 0; i < S && SAFE; i++) {
            Branch *c = back(into[i]), *d = front(into[i + 1]);
            if (c && d && c->slope >= d->slope) {
                d->adjust_width(c->width);
                into[i + 1] = splay(d);
                delete_back(into[i]);
            }
        }
        a = into[0];
        for (int i = 1; i <= S; i++) {
            a = join(a, into[i]);
        }
    }

    // Trim the left end of the tree by given length
    static void trim_front(Branch*& tree, V amount) {
        if (amount <= 0) {
            return;
        } else if (amount >= get_sum(tree)) {
            delete_all(tree), tree = nullptr;
        } else {
            auto node = find_by_rank(tree, amount);
            delete_exclusive(tree, nullptr, node);
        }
    }

    // Trim the right end of the tree by given length
    static void trim_back(Branch*& tree, V amount) {
        if (amount <= 0) {
            return;
        } else if (amount >= get_sum(tree)) {
            delete_all(tree), tree = nullptr;
        } else {
            auto node = find_by_rank(tree, get_sum(tree) - amount);
            delete_range(tree, node, nullptr);
        }
    }

    // Trim branch to [L,R) of current domain
    static void trimto(Branch*& tree, V L, V R) {
        L = max<V>(L, 0), R = min<V>(R, get_sum(tree));
        if (R <= L) {
            delete_all(tree), tree = nullptr;
        } else {
            trim_back(tree, get_sum(tree) - R), trim_front(tree, L);
        }
    }
};

// Branch wrapper that represents the complete function, tracking vertex and value there
struct Slopy {
    using V = Branch::V;
    static constexpr V inf = numeric_limits<V>::max() / 4;
    Branch* tree = nullptr;
    V v = 0, y = 0; // vertex, y value there

    // f(x) = {0 for x=0}
    Slopy() = default;

    void clear() { delete_all(tree), tree = nullptr, v = y = 0; }

    static Slopy point(V x, V y) {
        Slopy fn;
        fn.v = x, fn.y = y;
        return fn;
    }

    // f(x) = {y for L<=x<=R}
    static Slopy constant(V L, V R, V y) {
        assert(L <= R);
        Slopy fn;
        fn.v = L, fn.y = y;
        Branch::insert_segment(fn.tree, 0, R - L);
        return fn;
    }

    // f(x) = {a(x-L)+y for L<=x<=R}
    static Slopy slope(V a, V L, V R, V y) {
        assert(L <= R);
        Slopy fn;
        fn.v = L, fn.y = y;
        Branch::insert_segment(fn.tree, a, R - L);
        return fn;
    }

    // f(x) = ax+b for L<=x<=R
    static Slopy linear(V a, V b, V L, V R) {
        assert(L <= R);
        Slopy fn;
        fn.v = L, fn.y = a * L + b;
        Branch::insert_segment(fn.tree, a, R - L);
        return fn;
    }

    // f(x) = {a(A-x)+y if L<=x<=A | y if A<=x<=B | b(x-B) if B<=x<=R | for L<=x<=R}
    static Slopy valley(V a, V b, V L, V A, V B, V R, V y) {
        assert(a >= 0 && b >= 0 && L <= A && A <= B && B <= R);
        Slopy fn;
        fn.v = L, fn.y = a * (A - L) + y;
        Branch::insert_segment(fn.tree, -a, A - L);
        Branch::insert_segment(fn.tree, 0, B - A);
        Branch::insert_segment(fn.tree, b, R - B);
        return fn;
    }

    // f(x) = {a(M-x)+y if x<=M | b(x-M)+y if M<=x | for L<=x<=R}
    static Slopy vee(V a, V b, V L, V M, V R, V y) {
        assert(a + b >= 0 && L <= R); // convexity requirements
        Slopy fn;
        fn.v = L;
        if (L <= M && M <= R) {
            fn.y = a * (M - L) + y;
            Branch::insert_segment(fn.tree, -a, M - L);
            Branch::insert_segment(fn.tree, +b, R - M);
        } else if (M < L) {
            fn.y = b * (L - M) + y;
            Branch::insert_segment(fn.tree, +b, R - L);
        } else /* R < M */ {
            fn.y = a * (M - L) + y;
            Branch::insert_segment(fn.tree, -a, R - L);
        }
        return fn;
    }

    // merge g into f. f(x)=min{y+z=x}(a(y)+b(z))
    static void minplus(Slopy& fn, Slopy& gn) {
        fn.y += gn.y, fn.v += gn.v, Branch::minplus(fn.tree, gn.tree);
    }
    static void minplus(Slopy& fn, Slopy&& gn) { minplus(fn, gn); }

    // merge g into f. f(x)=g(x)+h(x) restricted to the intersected domain
    static void pointwise(Slopy& fn, Slopy& gn) {
        V vertex = max(fn.v, gn.v);
        V right = min(fn.right(), gn.right());
        assert(vertex <= right); // function domain is not empty
        fn.y = fn.query(vertex) + gn.query(vertex);
        Branch::pointwise(fn.tree, gn.tree, fn.v, gn.v);
        fn.v = vertex;
    }
    static void pointwise(Slopy& fn, Slopy&& gn) { pointwise(fn, gn); }

    // f(x) := g(x+c)
    void shift(V c) { v -= c; }

    // f(x) := g(x)+c
    void add_constant(V c) { y += c; }

    // f(x) := min{a+x<=y<=b+x} g(y)
    void range_min(V a, V b) { Branch::insert_segment(tree, 0, b - a), v -= b; }

    // f(x) := min{x<=y} g(y). Clears the left branch.
    void suffix_min() {
        auto [a, b] = split_key(tree, 0);
        auto len = Branch::get_sum(a);
        tree = b;
        delete_all(a);
        Branch::insert_segment(tree, 0, len);
    }

    // f(x) := min{y<=x} g(y). Clears the right branch.
    void prefix_min() {
        auto [a, b] = split_key(tree, 1);
        auto len = Branch::get_sum(b);
        tree = a;
        delete_all(b);
        Branch::insert_segment(tree, 0, len);
    }

    void trimto(V L, V R) {
        L = max(L, left()), R = min(R, right());
        assert(L <= R);
        y = query(L);
        Branch::trimto(tree, L - v, R - v);
        v = L;
    }

    void invert() { v = -right(), Branch::invert(tree); }

    V minimum() { return y + Branch::query(tree, Branch::slope_rank(tree, 0)); }
    V left_argmin() { return v + Branch::slope_rank(tree, 0); }
    V right_argmin() { return v + Branch::slope_rank(tree, 1); }
    V left() const { return v; }
    V right() const { return v + Branch::get_sum(tree); }

    bool within_domain(V x) const { return v <= x && x <= v + Branch::get_sum(tree); }

    V query(V x) { return within_domain(x) ? y + Branch::query(tree, x - v) : inf; }
    auto ceiling_range(V ceil) { return Branch::ceiling_range(tree, ceil - v); }
};
