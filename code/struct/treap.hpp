#pragma once

#include <bits/stdc++.h>
using namespace std;

// A treap with key operations (get_key()), order statistics (get_size()) and
// lazy propagation/aggregation, but no persistency, prev, next or parent pointers.
// Note that you should only call treap operations like join/split on treap roots!
template <typename Treap>
struct basic_treap {
    using uniform_priod = uniform_int_distribution<uint32_t>;
    static inline mt19937 rng = mt19937(random_device{}());
    static inline uniform_priod priod = uniform_priod(0, UINT32_MAX);

    Treap* kids[2] = {};
    uint32_t priority = priod(rng);

  protected:
    basic_treap() : priority(priod(rng)) {}

    Treap* self() { return static_cast<Treap*>(this); }
    const Treap* self() const { return static_cast<const Treap*>(this); }

  private:
    using TwoTreaps = pair<Treap*, Treap*>;
    using ThreeTreaps = tuple<Treap*, Treap*, Treap*>;

    static int get_size(const Treap* x) { return Treap::get_size(x); }
    static auto get_key(const Treap* x) { return Treap::get_key(x); }

  public:
    Treap* clone() const {
        auto node = new Treap(*self());
        node->priority = priod(rng), node->kids[0] = node->kids[1] = nullptr;
        return node;
    }

    friend void delete_all(Treap* u) {
        if (u) {
            delete_all(u->kids[0]);
            delete_all(u->kids[1]);
            delete u;
        }
    }

    // Merge two treaps in order, returns the new root
    friend Treap* join(Treap* a, Treap* b) {
        if (!a || !b) {
            return !b ? a : b;
        }
        if (a->priority >= b->priority) {
            a->pushdown();
            a->kids[1] = join(a->kids[1], b);
            a->pushup();
            return a;
        } else {
            b->pushdown();
            b->kids[0] = join(a, b->kids[0]);
            b->pushup();
            return b;
        }
    }

    // Merge three treaps in order, returns the new root
    friend Treap* join(Treap* a, Treap* b, Treap* c) { return join(join(a, b), c); }

    friend Treap* push_back(Treap* root, Treap* item) {
        if (!root) {
            return item;
        }
        root->pushdown();
        if (root->priority < item->priority) {
            item->kids[0] = join(root, item->kids[0]);
            item->pushup();
            return item;
        } else {
            root->kids[1] = push_back(root->kids[1], item);
            root->pushup();
            return root;
        }
    }

    friend Treap* push_front(Treap*& root, Treap* item) {
        if (!root) {
            return item;
        }
        root->pushdown();
        if (root->priority < item->priority) {
            item->kids[1] = join(item->kids[1], root);
            item->pushup();
            return item;
        } else {
            root->kids[0] = push_front(root->kids[0], item);
            root->pushup();
            return root;
        }
    }

    // Pop element from the back of the treap. Returns {new root, popped element}
    friend TwoTreaps pop_back(Treap* root) {
        if (!root) {
            return {};
        }
        root->pushdown();
        if (!root->kids[1]) {
            Treap* kid = root->kids[0];
            root->kids[0] = nullptr;
            return {kid, root};
        } else {
            Treap* back;
            tie(root->kids[1], back) = pop_back(root->kids[1]);
            root->pushup();
            return {root, back};
        }
    }

    // Pop element from the front of the treap. Returns {new root, popped element}
    friend TwoTreaps pop_front(Treap* root) {
        if (!root) {
            return {};
        }
        root->pushdown();
        if (!root->kids[0]) {
            Treap* kid = root->kids[1];
            root->kids[1] = nullptr;
            return {kid, root};
        } else {
            Treap* front;
            tie(root->kids[0], front) = pop_front(root->kids[0]);
            root->pushup();
            return {root, front};
        }
    }

    friend Treap* delete_back(Treap* root) {
        auto [ans, last] = pop_back(root);
        delete last;
        return ans;
    }

    friend Treap* delete_front(Treap* root) {
        auto [ans, first] = pop_front(root);
        delete first;
        return ans;
    }

    template <typename Vis>
    friend void beats_visit(Treap* root, Vis&& vis) {
        if (!root || vis(root)) {
            return;
        }
        root->pushdown();
        beats_visit(root->kids[0], vis);
        beats_visit(root->kids[1], vis);
        root->pushup();
    }

    // ***** Order operations

    // Find the item with this exact order, or nullptr if order is out of bounds
    friend Treap* find_order(Treap* root, int order) {
        if (!root) {
            return nullptr;
        }
        root->pushdown();
        int rorder = get_size(root->kids[0]);
        if (rorder > order) {
            return find_order(root->kids[0], order);
        } else if (rorder < order) {
            return find_order(root->kids[1], order - rorder - 1);
        } else {
            return root;
        }
    }

    // Insert item just before the given order (accepts out of bounds orders)
    friend Treap* insert_order(Treap* root, Treap* item, int order) {
        if (!root) {
            return item;
        }
        root->pushdown();
        int rorder = get_size(root->kids[0]);
        if (root->priority < item->priority) {
            auto [a, b] = split_order(root, order);
            item->pushdown();
            item->kids[0] = join(a, item->kids[0]);
            item->kids[1] = join(item->kids[1], b);
            item->pushup();
            return item;
        } else if (rorder >= order) {
            root->kids[0] = insert_order(root->kids[0], item, order);
            root->pushup();
            return root;
        } else {
            root->kids[1] = insert_order(root->kids[1], item, order - rorder - 1);
            root->pushup();
            return root;
        }
    }

    // Split into two treaps: {node<order} {order<=node}
    friend TwoTreaps split_order(Treap* root, int order) {
        if (!root || order <= 0) {
            return {nullptr, root};
        }
        root->pushdown();
        int rorder = get_size(root->kids[0]);
        Treap* a = {};
        if (rorder >= order) {
            tie(a, root->kids[0]) = split_order(root->kids[0], order);
            root->pushup();
            return {a, root};
        } else {
            tie(root->kids[1], a) = split_order(root->kids[1], order - rorder - 1);
            root->pushup();
            return {root, a};
        }
    }

    // Split into three treaps: {node<start} {start<=node<end} {end<=node}
    friend ThreeTreaps split_order_range(Treap* root, int start, int end) {
        auto [mid, c] = split_order(root, end);
        auto [a, b] = split_order(mid, start);
        return make_tuple(a, b, c);
    }

    // Splice (remove but not delete) item with given order. Returns {new root, spliced}
    friend TwoTreaps splice_order(Treap* root, int order) {
        if (!root) {
            return {};
        }
        root->pushdown();
        int rorder = get_size(root->kids[0]);
        Treap* ans;
        if (rorder > order) {
            tie(root->kids[0], ans) = splice_order(root->kids[0], order);
            root->pushup();
        } else if (rorder < order) {
            tie(root->kids[1], ans) = splice_order(root->kids[1], order - rorder - 1);
            root->pushup();
        } else {
            ans = root, root = join(root->kids[0], root->kids[1]);
            ans->kids[0] = ans->kids[1] = nullptr;
            ans->pushup();
        }
        return {root, ans};
    }

    friend Treap* delete_order(Treap* root, int order) {
        auto [new_root, ans] = splice_order(root, order);
        delete ans;
        return new_root;
    }

    friend Treap* delete_order_range(Treap* root, int start, int end) {
        auto [a, b, c] = split_order_range(root, start, end);
        delete_all(b);
        return join(a, c);
    }

    template <typename Vis>
    friend void beats_visit_order(Treap* root, Vis&& vis, int order = 0) {
        if (!root || vis(root, order, order + get_size(root))) {
            return;
        }
        root->pushdown();
        beats_visit(root->kids[0], vis, order);
        beats_visit(root->kids[1], vis, order + get_size(root->kids[0]) + 1);
        root->pushup();
    }

    // ***** Key operations

    template <typename Key>
    friend int order_of_key(Treap* root, const Key& key) {
        if (!root) {
            return 0;
        }
        root->pushdown();
        int rorder = get_size(root->kids[0]);
        if (get_key(root) < key) {
            return order_of_key(root->kids[1], key) + rorder + 1;
        } else if (key < get_key(root)) {
            return order_of_key(root->kids[0], key);
        } else {
            return rorder;
        }
    }

    // Find first item s.t. item->key >= key, or nullptr for end (lower_bound)
    template <typename Key>
    friend Treap* after(Treap* root, const Key& key) {
        Treap* ahead = nullptr;
        while (root) {
            root->pushdown();
            if (!(get_key(root) < key)) {
                ahead = root;
                root = root->kids[0];
            } else {
                root = root->kids[1];
            }
        }
        return ahead;
    }

    // Find first item s.t. item->key > key, or nullptr for end (upper_bound)
    template <typename Key>
    friend Treap* strict_after(Treap* root, const Key& key) {
        Treap* ahead = nullptr;
        while (root) {
            root->pushdown();
            if (key < get_key(root)) {
                ahead = root, root = root->kids[0];
            } else {
                root = root->kids[1];
            }
        }
        return ahead;
    }

    // Find last item s.t. item->key <= key, or nullptr for end (reverse lower_bound)
    template <typename Key>
    friend Treap* before(Treap* root, const Key& key) {
        Treap* ahead = nullptr;
        while (root) {
            root->pushdown();
            if (!(key < get_key(root))) {
                ahead = root, root = root->kids[1];
            } else {
                root = root->kids[0];
            }
        }
        return ahead;
    }

    // Find last item s.t. item->key < key, or nullptr for end (reverse upper_bound)
    template <typename Key>
    friend Treap* strict_before(Treap* root, const Key& key) {
        Treap* ahead = nullptr;
        while (root) {
            root->pushdown();
            if (get_key(root) < key) {
                ahead = root, root = root->kids[1];
            } else {
                root = root->kids[0];
            }
        }
        return ahead;
    }

    // Find first item s.t. item->key = key
    template <typename Key>
    friend Treap* find_key(Treap* root, const Key& key) {
        while (root) {
            root->pushdown();
            if (key < get_key(root)) {
                root = root->kids[0];
            } else if (get_key(root) < key) {
                root = root->kids[1];
            } else {
                return root;
            }
        }
        return nullptr;
    }

    // Insert item(s) in search position, returning the item splayed
    friend Treap* insert_key(Treap* root, Treap* item) {
        if (!root) {
            return item;
        }
        root->pushdown();
        if (root->priority < item->priority) {
            auto [a, b] = split_key(root, get_key(item));
            item->pushdown();
            item->kids[0] = join(a, item->kids[0]);
            item->kids[1] = join(item->kids[1], b);
            item->pushup();
            return item;
        } else if (get_key(root) < get_key(item)) {
            root->kids[1] = insert_key(root->kids[1], item);
            root->pushup();
            return root;
        } else {
            root->kids[0] = insert_key(root->kids[0], item);
            root->pushup();
            return root;
        }
    }

    template <typename Key>
    friend TwoTreaps split_key(Treap* root, const Key& key) {
        if (!root) {
            return {nullptr, nullptr};
        }
        root->pushdown();
        Treap* a = {};
        if (get_key(root) < key) {
            tie(root->kids[1], a) = split_key(root->kids[1], key);
            root->pushup();
            return {root, a};
        } else {
            tie(a, root->kids[0]) = split_key(root->kids[0], key);
            root->pushup();
            return {a, root};
        }
    }

    template <typename Key>
    friend ThreeTreaps split_key_range(Treap* root, const Key& L, const Key& R) {
        auto [mid, c] = split_key(root, R);
        auto [a, b] = split_key(mid, L);
        return make_tuple(a, b, c);
    }

    // Splice (remove but not delete) item with given key. Returns {new root, spliced}
    template <typename Key>
    friend TwoTreaps splice_key(Treap* root, const Key& key) {
        if (!root) {
            return {};
        }
        root->pushdown();
        Treap* ans;
        if (key < get_key(root)) {
            tie(root->kids[0], ans) = splice_key(root->kids[0], key);
            root->pushup();
        } else if (get_key(root) < key) {
            tie(root->kids[1], ans) = splice_key(root->kids[1], key);
            root->pushup();
        } else {
            ans = root, root = join(root->kids[0], root->kids[1]);
            ans->kids[0] = ans->kids[1] = nullptr;
            ans->pushup();
        }
        return {root, ans};
    }

    template <typename Key>
    friend Treap* delete_key(Treap* root, const Key& key) {
        auto [new_root, ans] = splice_key(root, key);
        delete ans;
        return new_root;
    }

    template <typename Key>
    friend Treap* delete_key_range(Treap* root, const Key& L, const Key& R) {
        auto [a, b, c] = split_key_range(root, L, R);
        delete_all(b);
        return join(a, c);
    }

    friend Treap* meld(Treap* A, Treap* B) {
        if (!A || !B) {
            return A ? A : B;
        }
        if (A->priority < B->priority) {
            swap(A, B);
        }
        A->pushdown(), B->pushdown();
        auto [L, R] = split_key(B, get_key(A));
        A->kids[0] = meld(L, A->kids[0]);
        A->kids[1] = meld(R, A->kids[1]);
        A->pushup();
        return A;
    }

    friend Treap* meld(Treap* A, Treap* B, Treap* C) { return meld(meld(A, B), C); }

    // ***** Root operations (delete on dfs)

    friend Treap* pop_root(Treap* root) {
        if (!root) {
            return root;
        }
        auto [L, R] = root->kids;
        root->kids[0] = root->kids[1] = nullptr;
        return join(L, R);
    }

    friend Treap* delete_root(Treap* root) {
        if (!root) {
            return nullptr;
        }
        auto [L, R] = root->kids;
        delete root;
        return join(L, R);
    }

    // ***** Debugging/auxiliary

    friend vector<Treap*> unstitch(Treap* A) {
        vector<Treap*> nodes = get_inorder_vector(A);
        for (Treap* node : nodes) {
            node->parent = node->kids[0] = node->kids[1] = nullptr;
            node->pushup();
        }
        return nodes;
    }

    friend Treap* stitch_dfs(const vector<Treap*>& nodes, int L, int R, Treap* parent) {
        if (L >= R) {
            return nullptr;
        }
        int M = (L + R) / 2;
        Treap* root = nodes[M];
        root->parent = parent;
        root->kids[0] = stitch_dfs(nodes, L, M, root);
        root->kids[1] = stitch_dfs(nodes, M + 1, R, root);
        root->pushup();
        return root;
    }

    friend Treap* stitch(const vector<Treap*>& nodes) {
        return stitch_dfs(nodes, 0, nodes.size(), nullptr);
    }

    friend vector<Treap*> get_inorder_vector(Treap* node) {
        vector<Treap*> vec;
        visit_inorder(node, [&vec](Treap* x) { vec.push_back(x); });
        return vec;
    }

    template <typename Fn>
    friend void visit_inorder(Treap* node, Fn&& fn) {
        if (node) {
            node->pushdown();
            auto [a, b] = node->kids;
            visit_inorder(a, fn);
            fn(node);
            visit_inorder(b, fn);
        }
    }

    friend string format_inorder(Treap* node) {
        int id = 0;
        string s;
        visit_inorder(node, [&](Treap* child) {
            s += " [" + to_string(id++) + "] " + child->format() + '\n';
        });
        return s;
    }
};

// Remember: updates should generally maintain that the current node is up-to-date (does
// not need pushup) but may need pushdown.
struct Treap : basic_treap<Treap> {
    int size = 1;
    int64_t key;
    int64_t sum = 0;
    int64_t lazy = 0;

    explicit Treap(int key, int value = 0) : key(key), sum(value) {}

    static int get_size(const Treap* x) { return x ? x->size : 0; }
    static auto get_key(const Treap* x) { return x->key; }
    static int64_t get_sum(const Treap* x) { return x ? x->sum : 0; }

    void update_self(int64_t add) {
        key += add;
        sum += add;
    }

    void update_range(int64_t add) {
        key += add;
        lazy += add;
        sum += 1LL * add * size;
    }

    void pushdown() {
        if (lazy) {
            if (kids[0])
                kids[0]->update_range(lazy);
            if (kids[1])
                kids[1]->update_range(lazy);
            lazy = 0;
        }
    }

    void pushup() {
        sum = key + get_sum(kids[0]) + get_sum(kids[1]);
        size = 1 + get_size(kids[0]) + get_size(kids[1]);
    }
};

/**
 * An interval tree with remove-on-stab functionality
 * Maintain a set of segments [l,r) and answer stab queries at i, removing the
 * segments that are found from the treap so they only get stabbed once. O(n log n).
 */
struct OnceStabTreap : basic_treap<OnceStabTreap> {
    struct Range {
        int L, R;
        bool operator<(Range o) const { return make_pair(L, R) < make_pair(o.L, o.R); }
        bool within(int i) const { return L <= i && i < R; }
    };
    Range key;
    int max_right = 0;

    explicit OnceStabTreap(Range key) : key(key), max_right(key.R) {}

    static auto get_key(const OnceStabTreap* x) { return x->key; }
    static int get_right(const OnceStabTreap* x) { return x ? x->max_right : 0; }

    void pushdown() {}

    void pushup() { max_right = max({key.R, get_right(kids[0]), get_right(kids[1])}); }

    static auto stab_query(OnceStabTreap*& root, int i) {
        vector<OnceStabTreap*> rs;
        root = stab_query_dfs(root, i, rs);
        return rs;
    }

    static OnceStabTreap* stab_query_dfs(OnceStabTreap* x, int i,
                                         vector<OnceStabTreap*>& rs) {
        while (x && x->key.within(i)) {
            rs.push_back(x);
            x = pop_root(x);
        }
        if (x && get_right(x->kids[0]) > i) {
            x->kids[0] = stab_query_dfs(x->kids[0], i, rs);
            x->pushup();
        }
        if (x && get_right(x->kids[1]) > i && x->key.L <= i) {
            x->kids[1] = stab_query_dfs(x->kids[1], i, rs);
            x->pushup();
        }
        return x;
    }
};

/**
 * An interval tree with single-stab functionality
 * Maintain a set of segments [l,r) and answer stab queries at i.
 * The stabbed node is not popped.
 * The user guarantees that there is exactly zero or one stabbed intervals. O(n log n).
 */
struct SingleStabTreap : basic_treap<SingleStabTreap> {
    struct Range {
        int L, R;
        bool operator<(Range o) const { return make_pair(L, R) < make_pair(o.L, o.R); }
        bool within(int i) const { return L <= i && i < R; }
    };
    Range key;
    int max_right = 0;

    explicit SingleStabTreap(Range key) : key(key), max_right(key.R) {}

    static auto get_key(const SingleStabTreap* x) { return x->key; }
    static int get_right(const SingleStabTreap* x) { return x ? x->max_right : 0; }

    void pushdown() {}

    void pushup() { max_right = max({key.R, get_right(kids[0]), get_right(kids[1])}); }

    static auto stab_query(SingleStabTreap*& root, int i) {
        SingleStabTreap* ans = nullptr;
        root = stab_query_dfs(root, i, ans);
        return ans;
    }

    static SingleStabTreap* stab_query_dfs(SingleStabTreap* x, int i,
                                           SingleStabTreap*& ans) {
        if (x && x->key.within(i)) {
            ans = x;
        } else if (x && get_right(x->kids[0]) > i) {
            x->kids[0] = stab_query_dfs(x->kids[0], i, ans);
            x->pushup();
        } else if (x && get_right(x->kids[1]) > i && x->key.L <= i) {
            x->kids[1] = stab_query_dfs(x->kids[1], i, ans);
            x->pushup();
        }
        return x;
    }
};

/**
 * Maintain key->value mappings, support setmin and getmin on range values. O(n log n)
 */
struct MinTreap : basic_treap<MinTreap> {
    static constexpr int64_t MAX = numeric_limits<int64_t>::max() / 2;

    int size = 1;
    int key;
    int64_t value = MAX;
    int64_t minimum = MAX;
    int64_t set_lazy = MAX;

    explicit MinTreap(int key, int64_t value) : key(key), value(value), minimum(value) {}

    static int get_size(const MinTreap* x) { return x ? x->size : 0; }
    static auto get_key(const MinTreap* x) { return x->key; }
    static int64_t get_min(const MinTreap* x) { return x ? x->minimum : MAX; }

    void apply_setmin(int64_t setmin) {
        value = min(value, setmin);
        minimum = min(minimum, setmin);
        set_lazy = min(set_lazy, setmin);
    }

    void pushdown() {
        if (kids[0])
            kids[0]->apply_setmin(set_lazy);
        if (kids[1])
            kids[1]->apply_setmin(set_lazy);
        set_lazy = MAX;
    }

    void pushup() {
        minimum = min({value, get_min(kids[0]), get_min(kids[1])});
        size = 1 + get_size(kids[0]) + get_size(kids[1]);
    }

    static MinTreap* query_remove_min(MinTreap* x, MinTreap*& node) {
        x->pushdown();
        if (x->value == x->minimum) {
            node = x;
            x = join(x->kids[0], x->kids[1]);
            node->kids[0] = node->kids[1] = nullptr, node->pushup();
        } else if (get_min(x->kids[0]) == x->minimum) {
            x->kids[0] = query_remove_min(x->kids[0], node);
            x->pushup();
        } else if (get_min(x->kids[1]) == x->minimum) {
            x->kids[1] = query_remove_min(x->kids[1], node);
            x->pushup();
        } else {
            assert(false);
        }
        return x;
    }
};

/**
 * Splay supporting the following operations:
 * Insert range of integers [l,r), none existing (delete first if you need to be sure)
 * Delete range of integers [l,r), some might not exist
 * Query k-th integer
 */
struct RangeTreap : basic_treap<RangeTreap> {
    using V = int64_t;
    V length, L, R;

    explicit RangeTreap(V L, V R) : length(R - L), L(L), R(R) {}

    static auto get_key(const RangeTreap* x) { return x->L; }
    static V get_length(const RangeTreap* x) { return x ? x->length : 0; }

#ifdef LOCAL
    string format() const { return '(' + to_string(L) + ' ' + to_string(R) + ')'; }
#endif

    void pushdown() {}

    void pushup() { length = R - L + get_length(kids[0]) + get_length(kids[1]); }

    friend RangeTreap* within_range_find(RangeTreap*& tree, V i) {
        RangeTreap* node = tree;
        RangeTreap* after = nullptr;
        while (node) {
            if (i < node->L) {
                after = node;
                node = node->kids[0];
            } else if (node->R <= i) {
                node = node->kids[1];
            } else {
                return node;
            }
        }
        return after;
    }

    friend void clear_range(RangeTreap*& tree, V l, V r) {
        auto right = within_range_find(tree, r);
        if (right && right->L < r && r < right->R) {
            split_node_proper(tree, right, r);
        }
        auto left = within_range_find(tree, l);
        if (left && left->L < l && l < left->R) {
            split_node_proper(tree, left, l);
        }
        tree = delete_key_range(tree, l, r);
    }

    friend void insert_range(RangeTreap*& tree, V l, V r) {
        tree = insert_key(tree, new RangeTreap(l, r));
    }

    friend void split_node_proper(RangeTreap*& tree, RangeTreap* x, V M) {
        assert(x->L < M && M < x->R);
        RangeTreap* y = new RangeTreap(M, x->R);
        tie(tree, x) = splice_key(tree, x->L);
        x->R = M, x->pushup();
        tree = insert_key(tree, x);
        tree = insert_key(tree, y);
    }

    friend V range_kth(RangeTreap*& tree, V k) {
        RangeTreap* node = tree;
        assert(0 <= k && k < get_length(tree));
        while (true) {
            if (k < get_length(node->kids[0])) {
                node = node->kids[0];
            }
            k -= get_length(node->kids[0]);
            if (k < get_length(node)) {
                return node->L + k;
            }
            k -= get_length(node);
            node = node->kids[1];
        }
    }

    friend V range_rank(RangeTreap*& tree, V i) {
        int64_t rank = 0;
        RangeTreap* node = tree;
        while (node) {
            if (i < node->L) {
                node = node->kids[0];
            } else if (node->R <= i) {
                rank += node->R - node->L + get_length(node->kids[0]);
                node = node->kids[1];
            } else {
                rank += get_length(node->kids[0]);
                return rank + (i - node->L);
            }
        }
        return rank;
    }
};
