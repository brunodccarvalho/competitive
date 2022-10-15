#pragma once

#include <bits/stdc++.h>
using namespace std;

// A treap with key operations (get_key()), order statistics (get_size()) and
// lazy propagation/aggregation, but no persistency, prev, next or parent pointers.
// Note that you should only call treap operations like join/split on treap roots!
template <typename Treap>
struct functional_treap {
    using uniform_priod = uniform_int_distribution<uint32_t>;
    static inline mt19937 rng = mt19937(random_device{}());
    static inline uniform_priod priod = uniform_priod(0, UINT32_MAX);

    using STreap = shared_ptr<Treap>;
    STreap kids[2] = {};
    uint32_t priority = priod(rng);

    static int get_size(STreap x) { return x ? x->size : 0; }
    static const auto& get_key(STreap x) { return x->key; }

  protected:
    functional_treap() : priority(priod(rng)) {}
    inline void pushdown() {}
    inline void pushup() {}

    Treap* self() { return static_cast<Treap*>(this); }
    const Treap* self() const { return static_cast<const Treap*>(this); }

    void persistent_pushdown() {
        if (kids[0])
            kids[0] = make_shared<Treap>(*kids[0]);
        if (kids[1])
            kids[1] = make_shared<Treap>(*kids[1]);
    }

    STreap persistent_clone() const { return make_shared<Treap>(*self()); }

  private:
    using TwoTreaps = pair<STreap, STreap>;
    using ThreeTreaps = tuple<STreap, STreap, STreap>;

  public:
    STreap clone() const {
        auto node = make_shared<Treap>(*self());
        node->priority = priod(rng), node->kids[0] = node->kids[1] = nullptr;
        return node;
    }

    // Merge two treaps in order
    friend STreap join(STreap a, STreap b) {
        if (!a || !b) {
            return !b ? a : b;
        }
        if (a->priority >= b->priority) {
            a->pushdown(), a = a->persistent_clone();
            a->kids[1] = join(a->kids[1], b);
            a->pushup();
            return a;
        } else {
            b->pushdown(), b = b->persistent_clone();
            b->kids[0] = join(a, b->kids[0]);
            b->pushup();
            return b;
        }
    }

    // Merge three treaps in order, returns the new root
    friend STreap join(STreap a, STreap b, STreap c) { return join(join(a, b), c); }

    friend STreap push_back(STreap root, STreap item) {
        if (!root) {
            return item;
        }
        root->pushdown(), root = root->persistent_clone();
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

    friend STreap push_front(STreap& root, STreap item) {
        if (!root) {
            return item;
        }
        root->pushdown(), root = root->persistent_clone();
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
    friend TwoTreaps pop_back(STreap root) {
        if (!root) {
            return {};
        }
        root->pushdown(), root = root->persistent_clone();
        if (!root->kids[1]) {
            STreap kid = root->kids[0];
            root->kids[0] = nullptr;
            return {kid, root};
        } else {
            STreap back;
            tie(root->kids[1], back) = pop_back(root->kids[1]);
            root->pushup();
            return {root, back};
        }
    }

    // Pop element from the front of the treap. Returns {new root, popped element}
    friend TwoTreaps pop_front(STreap root) {
        if (!root) {
            return {};
        }
        root->pushdown(), root = root->persistent_clone();
        if (!root->kids[0]) {
            STreap kid = root->kids[1];
            root->kids[1] = nullptr;
            return {kid, root};
        } else {
            STreap front;
            tie(root->kids[0], front) = pop_front(root->kids[0]);
            root->pushup();
            return {root, front};
        }
    }

    friend STreap delete_back(STreap root) { return pop_back(root).first; }

    friend STreap delete_front(STreap root) { return pop_front(root).first; }

    // ***** Order operations

    // Find the item with this exact order, or nullptr if order is out of bounds
    friend STreap find_order(STreap root, int order) {
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
    friend STreap insert_order(STreap root, STreap item, int order) {
        if (!root) {
            return item;
        }
        root->pushdown(), root = root->persistent_clone();
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
            return root;
        }
    }

    // Split into two treaps: {node<order} {order<=node}
    friend TwoTreaps split_order(STreap root, int order) {
        if (!root || order <= 0) {
            return {nullptr, root};
        }
        root->pushdown(), root = root->persistent_clone();
        int rorder = get_size(root->kids[0]);
        STreap a = {};
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
    friend ThreeTreaps split_order_range(STreap root, int start, int end) {
        auto [mid, c] = split_order(root, end);
        auto [a, b] = split_order(mid, start);
        return make_tuple(a, b, c);
    }

    // Splice (remove but not delete) item with given order. Returns {new root, spliced}
    friend TwoTreaps splice_order(STreap root, int order) {
        if (!root) {
            return {};
        }
        root->pushdown(), root = root->persistent_clone();
        int rorder = get_size(root->kids[0]);
        STreap ans;
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

    friend STreap delete_order(STreap root, int order) {
        return splice_order(root, order).first;
    }

    friend STreap delete_order_range(STreap root, int start, int end) {
        auto [a, b, c] = split_order_range(root, start, end);
        return join(a, c);
    }

    // ***** Key operations

    template <typename Key>
    friend int order_of_key(STreap root, const Key& key) {
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
    friend STreap after(STreap root, const Key& key) {
        STreap ahead = nullptr;
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
    friend STreap strict_after(STreap root, const Key& key) {
        STreap ahead = nullptr;
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
    friend STreap before(STreap root, const Key& key) {
        STreap ahead = nullptr;
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
    friend STreap strict_before(STreap root, const Key& key) {
        STreap ahead = nullptr;
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
    friend STreap find_key(STreap root, const Key& key) {
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

    // Insert item(s) in search position. Does not clone the item (do so yourself)
    friend STreap insert_key(STreap root, STreap item) {
        if (!root) {
            return item;
        }
        root->pushdown(), root = root->persistent_clone();
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
    friend TwoTreaps split_key(STreap root, const Key& key) {
        if (!root) {
            return {nullptr, nullptr};
        }
        root->pushdown(), root = root->persistent_clone();
        STreap a = {};
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
    friend ThreeTreaps split_key_range(STreap root, const Key& L, const Key& R) {
        auto [mid, c] = split_key(root, R);
        auto [a, b] = split_key(mid, L);
        return make_tuple(a, b, c);
    }

    // Splice (remove but not delete) item with given key. Returns {new root, spliced}
    template <typename Key>
    friend TwoTreaps splice_key(STreap root, const Key& key) {
        if (!root) {
            return {root, nullptr};
        }
        root->pushdown(), root = root->persistent_clone();
        STreap ans;
        if (key < get_key(root)) {
            tie(root->kids[0], ans) = splice_key(root->kids[0], key);
            root->pushup();
        } else if (get_key(root) < key) {
            tie(root->kids[1], ans) = splice_key(root->kids[1], key);
            root->pushup();
        } else {
            ans = root, root = join(root->kids[0], root->kids[1]);
            root->kids[0] = root->kids[1] = nullptr;
        }
        return {root, ans};
    }

    template <typename Key>
    friend STreap delete_key(STreap root, const Key& key) {
        return splice_key(root, key).first;
    }

    template <typename Key>
    friend STreap delete_key_range(STreap root, const Key& L, const Key& R) {
        auto [a, b, c] = split_key_range(root, L, R);
        return join(a, c);
    }

    friend STreap meld(STreap A, STreap B) {
        if (!A || !B) {
            return A ? A : B;
        }
        if (A->priority < B->priority) {
            swap(A, B);
        }
        A->pushdown(), B->pushdown();
        auto [L, R] = split_key(B, get_key(A));
        A = A->persistent_clone();
        A->kids[0] = meld(L, A->kids[0]);
        A->kids[1] = meld(R, A->kids[1]);
        A->pushup();
        return A;
    }

    friend STreap meld(STreap A, STreap B, STreap C) { return meld(meld(A, B), C); }

    // ***** Debugging/auxiliary

    template <typename Fn>
    friend void visit_inorder(STreap node, Fn&& fn) {
        if (node) {
            node->pushdown();
            auto [a, b] = node->kids;
            visit_inorder(a, fn);
            fn(node);
            visit_inorder(b, fn);
        }
    }

    friend string format_inorder(STreap node) {
        int id = 0;
        string s;
        visit_inorder(node, [&](STreap child) {
            s += " [" + to_string(id++) + "] " + child->format() + '\n';
        });
        return s;
    }
};

// Remember: updates should generally maintain that the current node is up-to-date (does
// not need pushup) but may need pushdown.
struct Treap : functional_treap<Treap> {
    int size = 1;
    int64_t key;
    int64_t sum = 0;
    int64_t lazy = 0;

    explicit Treap(int key) : key(key), sum(key) {}

    friend STreap make_treap(int key) { return make_shared<Treap>(key); }

    STreap update_self(int64_t add) {
        auto node = this->persistent_clone();
        node->key += add;
        node->sum += add;
        return node;
    }

    STreap update_range(int64_t add) {
        auto node = this->persistent_clone();
        node->key += add;
        node->lazy += add;
        node->sum += 1LL * add * size;
        return node;
    }

    void pushdown() {
        if (lazy) {
            this->persistent_pushdown();
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

    string format() const { return fmt::format("[{}] s={}", key, sum); }

    static int64_t get_sum(STreap x) { return x ? x->sum : 0; }
};
