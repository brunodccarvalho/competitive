#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <limits>

/**
 * AVL rotation notes
 *
 *       y                     x
 *      / \                   / \
 *    [a]  x        ->       y  [c]
 *        / \               / \
 *      [b] [c]           [a] [b]
 *
 * height(a) = h
 * height(b) = {h-1, h, h+1}
 * height(c) = {h-1, h, h+1}
 *
 * assert(balance(y) >= +1 and balance(y) >= balance(x))
 *
 * balance(y) == +1 {
 *   balance(x) == -1:
 *     heights: h, h, h-1
 *     new:    balance(y) = 0    balance(x) = -2  (!)
 *   balance(x) == 0:
 *     heights: h, h, h
 *     new:    balance(y) = 0    balance(x) = -1
 *   balance(x) == +1:
 *     heights: h, h-1, h
 *     new:    balance(y) = -1   balance(x) = -1
 * }
 *
 * balance(y) == +2 {
 *   balance(x) == -1:
 *     heights: h, h+1, h
 *     new:    balance(y) = +1   balance(x) = -2  (!)
 *   balance(x) == 0:
 *     heights: h, h+1, h+1
 *     new:    balance(y) = +1   balance(x) = -1
 *   balance(x) == +1:
 *     heights: h, h, h+1
 *     new:    balance(y) = 0    balance(x) = 0   (height reduced)
 *   balance(x) == +2:
 *     heights: h, h-1, h+1
 *     new:    balance(y) = -1   balance(x) = 0   (height reduced)
 * }
 *
 * Note: the height delta (difference in height compared to the previous tree)
 * is -1 (i.e. the height diminished) iff rotations occurred and the new root
 * is 0-balanced.
 *
 * AVL functions
 * We handle rotations and rebalancing modularly, namely:
 *   - y knows how to rebalance itself
 *   - rotation functions themselves maintain invariants on y and x
 *   - rotation functions compose, so that a left-right rotation can be literally coded as
 *     a right rotation followed by a left rotation and the result is what's expected.
 */

/**
 * AVL node
 * This same class is used to represent the head node. The node is the tree's head
 * iff it does not hold data iff the parent pointer is this (itself).
 */
template <typename T>
struct avl_node {
    using node_t = avl_node<T>;
    template <typename Tree>
    friend struct tree_debugger;

    node_t* parent = nullptr;
    node_t* link[2] = {};
    union {
        int8_t _dummy;
        T data;
    };
    int8_t balance = 0;

    ~avl_node() {
        delete link[0];
        delete link[1];
        if (this != parent)
            data.~T();
    }

    static node_t* minimum(node_t* n) noexcept {
        while (n->link[0])
            n = n->link[0];
        return n;
    }
    static const node_t* minimum(const node_t* n) noexcept {
        while (n->link[0])
            n = n->link[0];
        return n;
    }
    static node_t* maximum(node_t* n) noexcept {
        while (n->link[1])
            n = n->link[1];
        return n;
    }
    static const node_t* maximum(const node_t* n) noexcept {
        while (n->link[1])
            n = n->link[1];
        return n;
    }
    static node_t* increment(node_t* n) noexcept {
        if (n->link[1])
            return minimum(n->link[1]);
        while (n == n->parent->link[1])
            n = n->parent;
        return n->parent;
    }
    static const node_t* increment(const node_t* n) noexcept {
        if (n->link[1])
            return minimum(n->link[1]);
        while (n == n->parent->link[1])
            n = n->parent;
        return n->parent;
    }
    static node_t* decrement(node_t* n) noexcept {
        if (n->link[0])
            return maximum(n->link[0]);
        while (n == n->parent->link[0])
            n = n->parent;
        return n->parent;
    }
    static const node_t* decrement(const node_t* n) noexcept {
        if (n->link[0])
            return maximum(n->link[0]);
        while (n == n->parent->link[0])
            n = n->parent;
        return n->parent;
    }

  private:
    avl_node(const avl_node&) = delete;
    avl_node(avl_node&&) = delete;
    avl_node& operator=(const avl_node&) = delete;
    avl_node& operator=(avl_node&&) = delete;

    // hide this to prevent default-constructed data from creating head nodes
    struct avl_head_tag_t {};
    avl_node(avl_head_tag_t _tag) : parent(this) { (void)_tag; }

    avl_node(T data) : data(std::move(data)) {}
    template <typename... Args>
    avl_node(Args&&... args) : data(std::forward<Args>(args)...) {}

  public:
    static node_t* new_empty() { return new avl_node(avl_head_tag_t{}); }

    template <typename... Args>
    static node_t* make(Args&&... args) {
        return new avl_node(std::forward<Args>(args)...);
    }
};

/**
 * AVL binary search tree core
 * Tree is completely open
 */
template <typename T>
struct avl_tree {
    using node_t = avl_node<T>;
    template <typename Tree>
    friend struct tree_debugger;

    // The real tree's root is head->link[0]. head is never nullptr.
    node_t* head;
    node_t* min_node;
    node_t* max_node;
    size_t node_count;

    avl_tree() noexcept
        : head(node_t::new_empty()), min_node(head), max_node(head), node_count(0) {}

    // Move constructor
    avl_tree(avl_tree&& other) noexcept : avl_tree() { swap(other); }
    // Copy constructor
    avl_tree(const avl_tree& other) noexcept
        : head(node_t::new_empty()), node_count(other.node_count) {
        adopt_node(head, deep_clone_node(other.head->link[0]), 0);
        update_minmax();
    }
    // Move assignment
    avl_tree& operator=(avl_tree&& other) noexcept {
        if (head != other.head) {
            clear();
            swap(other);
        }
        return *this;
    }
    // Copy assignment
    avl_tree& operator=(const avl_tree& other) noexcept {
        if (head != other.head) {
            delete head->link[0];
            adopt_node(head, deep_clone_node(other.head->link[0]), 0);
            update_minmax();
            node_count = other.node_count;
        }
        return *this;
    }

    ~avl_tree() noexcept { delete head; }

    void clear() noexcept {
        delete head->link[0];
        head->link[0] = nullptr;
        min_node = max_node = head;
        node_count = 0;
    }
    void swap(avl_tree& other) noexcept {
        std::swap(head, other.head);
        std::swap(min_node, other.min_node);
        std::swap(max_node, other.max_node);
        std::swap(node_count, other.node_count);
    }
    friend void swap(avl_tree& lhs, avl_tree& rhs) noexcept { lhs.swap(rhs); }

    node_t* minimum() noexcept { return min_node; }
    const node_t* minimum() const noexcept { return min_node; }
    node_t* maximum() noexcept { return max_node; }
    const node_t* maximum() const noexcept { return max_node; }

  private:
    void update_minmax() {
        if (head->link[0]) {
            min_node = node_t::minimum(head->link[0]);
            max_node = node_t::maximum(head->link[0]);
        } else {
            min_node = max_node = head;
        }
    }
    static void drop_node(node_t* node) {
        node->link[0] = node->link[1] = nullptr;
        delete node;
    }
    static void adopt_node(node_t* parent, node_t* child, bool side) {
        parent->link[side] = child;
        if (child)
            child->parent = parent;
    }
    static void clear_node(node_t* node) {
        node->link[0] = node->link[1] = nullptr;
        node->parent = node;
        node->balance = 0;
    }
    static node_t* deep_clone_node(const node_t* node) {
        if (!node)
            return nullptr;
        node_t* clone = node_t::make(node->data);
        clone->balance = node->balance;
        adopt_node(clone, deep_clone_node(node->link[0]), 0);
        adopt_node(clone, deep_clone_node(node->link[1]), 1);
        return clone;
    }

    /**
     *       y                     x
     *      / \                   / \
     *    [a]  x        ->       y  [c]
     *        / \               / \
     *      [b] [c]           [a] [b]
     */
    node_t* rotate_left(node_t* y) {
        node_t* x = y->link[1];
        assert(y->balance >= +1 && y->balance >= x->balance);
        bool yside = y == y->parent->link[1];
        adopt_node(y->parent, x, yside);
        adopt_node(y, x->link[0], 1);
        adopt_node(x, y, 0);
        int xb = x->balance;
        int y1 = y->balance == +1;
        int y2 = y->balance == +2;
        y->balance = -std::max(xb - y2, -y2);
        x->balance = std::min(xb - 1, -y1);
        return x;
    }

    /**
     *         y                  x
     *        / \                / \
     *       x  [c]     ->     [a]  y
     *      / \                    / \
     *    [a] [b]                [b] [c]
     */
    node_t* rotate_right(node_t* y) {
        node_t* x = y->link[0];
        assert(y->balance <= -1 && y->balance <= x->balance);
        bool yside = y == y->parent->link[1];
        adopt_node(y->parent, x, yside);
        adopt_node(y, x->link[1], 0);
        adopt_node(x, y, 1);
        int xb = x->balance;
        int y1 = y->balance == -1;
        int y2 = y->balance == -2;
        y->balance = -std::min(xb + y2, y2);
        x->balance = std::max(xb + 1, y1);
        return x;
    }

    /**
     * Recalibrate the tree rooted at y that has become unbalanced, deducing
     * the necessary rotations. Does nothing if y is already balanced.
     * Returns the new root after calibration.
     */
    node_t* rebalance(node_t* y) {
        if (y->balance == -2) {
            if (y->link[0]->balance == +1) {
                rotate_left(y->link[0]);
            }
            return rotate_right(y);
        }
        if (y->balance == +2) {
            if (y->link[1]->balance == -1) {
                rotate_right(y->link[1]);
            }
            return rotate_left(y);
        }
        return y;
    }

    /**
     *             p(-1)                     p(0)                       p(+1)
     *  h+1->h    / \   h         h->h-1    / \    h         h-1->h-2  / \   h
     *           /   \                     /   \                      /   \
     *          l     r                   l     r                    l     r
     *         / \   / \                 / \   / \                  / \   / \
     *       -.. .. .. ..              -.. .. .. ..               -.. .. .. ..
     * height lower in left       height lower in left    height lower in left
     *  new balance: 0             new balance: +1         new balance: +2 -> rebalance!
     *  delta height: h+1->h       delta height: h->h      delta height: 0 if new root
     *  overall height decreased   overall height did not  is imperfectly balanced (stop)
     *  continue on p's parent.    change so stop.         otherwise -1 so continue.
     */
    void rebalance_after_erase(node_t* y) {
        if (y == head)
            return;
        y = rebalance(y);
        while (y->parent != head && y->balance == 0) {
            bool yside = y == y->parent->link[1];
            y->parent->balance += yside ? -1 : +1;
            y = rebalance(y->parent);
        }
    }

    /**
     *            p(+1)                     p(0)                        p(-1)
     * h-1->h    / \   h         h->h+1    / \    h          h+1->h+2  / \   h
     *          /   \                     /   \                       /   \
     *         l     r                   l     r                     l     r
     *        / \   / \                 / \   / \                   / \   / \
     *      +.. .. .. ..              +.. .. .. ..                +.. .. .. ..
     * height higher in left   height higher in left     height higher in left
     *  new balance: 0          new balance: -1           new balance: -2 -> rebalance!
     *  delta height: h->h      delta height: h->h+1      delta height: h+1->h+1 (always)
     *  overall height did not  overall height increased  overall height did not
     *  change so stop.         continue on p's parent    change so stop.
     */
    void rebalance_after_insert(node_t* y) {
        while (y->parent != head && y->parent->balance == 0) {
            bool yside = y == y->parent->link[1];
            y->parent->balance = yside ? +1 : -1;
            y = y->parent;
        }
        if (y->parent != head) {
            bool yside = y == y->parent->link[1];
            y->parent->balance += yside ? +1 : -1;
            rebalance(y->parent);
        }
    }

    /**
     *   parent       parent  <-- rebalance here
     *     |            |
     *     y     ->    [x]
     *    /
     *  [x]
     *                 balance(parent) := ±1
     */
    void erase_node_pull_left(node_t* y) {
        node_t* x = y->link[0];
        node_t* parent = y->parent;
        bool yside = y == y->parent->link[1];
        adopt_node(parent, x, yside);
        if (parent != head) {
            parent->balance += yside ? -1 : +1;
            rebalance_after_erase(parent);
        }
    }

    /**
     *     |            |
     *     y            x  <-- rebalance here
     *    / \    ->    / \
     *  [a]  x       [a] [b]
     *        \
     *        [b]
     *               balance(x) := balance(y) - 1
     */
    void erase_node_pull_right(node_t* y) {
        node_t* x = y->link[1];
        node_t* parent = y->parent;
        bool yside = y == parent->link[1];
        adopt_node(parent, x, yside);
        adopt_node(x, y->link[0], 0);
        x->balance = y->balance - 1;
        rebalance_after_erase(x);
    }

    /**
     *        |                       |
     *        y                       x
     *       / \                     / \
     *     [a]  x1                 [a]  x1
     *         / \                     / \
     *       ... [b]      ->         ... [b]
     *       / \                     / \
     *      w  [c]   rebalance -->  w  [c]
     *     / \          here       / \
     *    x  [d]                 [e]  [d]
     *     \
     *     [e]
     *                       balance(x) := balance(y)
     *                       balance(w) := balance(w) + 1
     */
    void erase_node_minimum(node_t* y) {
        node_t* x = node_t::minimum(y->link[1]->link[0]);
        node_t* w = x->parent;
        node_t* parent = y->parent;
        bool yside = y == parent->link[1];
        adopt_node(parent, x, yside);
        adopt_node(w, x->link[1], 0);
        adopt_node(x, y->link[0], 0);
        adopt_node(x, y->link[1], 1);
        x->balance = y->balance;
        w->balance += 1;
        rebalance_after_erase(w);
    }

    /**
     * Select the erase position based on y's right subtree
     */
    void erase_node_and_rebalance(node_t* y) {
        if (!y->link[1])
            erase_node_pull_left(y);
        else if (!y->link[1]->link[0])
            erase_node_pull_right(y);
        else
            erase_node_minimum(y);
    }

    /**
     * Update min max pointers before insert
     */
    void insert_minmax(node_t* parent, node_t* y, bool side) {
        if (node_count > 0) {
            if (!side && parent == min_node)
                min_node = y;
            else if (side && parent == max_node)
                max_node = y;
        } else {
            min_node = max_node = y;
        }
    }

    /**
     * Update min max pointers before erase
     */
    void erase_minmax(node_t* y) {
        if (node_count > 1) {
            if (y == min_node)
                min_node = y->link[1] ? y->link[1] : y->parent;
            else if (y == max_node)
                max_node = y->link[0] ? y->link[0] : y->parent;
        } else {
            min_node = max_node = head;
        }
    }

  public:
    /**
     * Insert node y into the tree as a child of parent on the given side.
     * parent must not have a child on that side and y must be a free node.
     *
     *    parent         parent                 parent        parent
     *     /       ->     /  \         or           \    ->    /  \
     *   [l]            [l]   y                     [r]       y   [r]
     */
    void insert_node(node_t* parent, node_t* y, bool side) {
        insert_minmax(parent, y, side);
        adopt_node(parent, y, side);
        rebalance_after_insert(y);
        node_count++;
    }

    /**
     * Insert node y after node, so that incrementing node afterwards gives y.
     * Usually this will insert y as the right child of node.
     * y must be a free node.
     *
     *    parent         parent                 parent        parent
     *     /       ->     /  \         or        /  \    ->    /  \
     *   [l]            [l]   y                [l]   r       [l]   r
     *                                              /             /
     *                                            ...           ...
     *                                            /             /
     *                              ++parent --> x             x
     *                                            \           / \
     *                                            [r]        y  [r]
     */
    void insert_node_after(node_t* node, node_t* y) {
        if (node->link[1]) {
            insert_node(node_t::increment(node), y, 0);
        } else {
            insert_node(node, y, 1);
        }
    }

    /**
     * Insert node y before node, so that decrementing node afterwards gives y.
     * Usually this will insert y as the left child of node.
     * y must be a free node.
     *
     *   parent        parent                  parent        parent
     *       \    ->    /  \          or        /  \    ->    /  \
     *       [r]       y   [r]                 l   [r]       l   [r]
     *                                          \             \
     *                                          ...           ...
     *                                            \             \
     *                                --parent --> x             x
     *                                            /             / \
     *                                          [l]           [l]  y
     */
    void insert_node_before(node_t* node, node_t* y) {
        if (node->link[0]) {
            insert_node(node_t::decrement(node), y, 1);
        } else {
            insert_node(node, y, 0);
        }
    }

    /**
     * Remove node y from the tree and destroy it.
     */
    void erase_node(node_t* y) {
        erase_minmax(y);
        erase_node_and_rebalance(y);
        drop_node(y);
        node_count--;
    }

    /**
     * Remove node y from the tree but do not destroy it.
     */
    void yank_node(node_t* y) {
        erase_minmax(y);
        erase_node_and_rebalance(y);
        clear_node(y);
        node_count--;
    }
};
