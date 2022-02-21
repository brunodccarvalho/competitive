#pragma once

#include <cassert>
#include <functional>
#include <limits>

/**
 * RB rotation notes
 *
 * Insert:
 *   https://www.youtube.com/watch?v=O3hI9FdxFOM
 *   pseudo-code is nice
 *
 * Erase:
 *   https://cs.kangwon.ac.kr/~leeck/file_processing/red_black_tree.pdf
 *   code is awful but it works and explanation is ok
 *
 * CLRS: nice explanations, code is ok
 */

enum rb_color_t : int8_t { rb_red = 0, rb_black = 1 };

/**
 * Red-black node
 * This same class is used to represent the head node. The node is the tree's head
 * iff it does not hold data iff the parent pointer is this (itself).
 */
template <typename T>
struct rb_node {
    using node_t = rb_node<T>;

    node_t* parent = nullptr;
    node_t* link[2] = {};
    union {
        int8_t _dummy;
        T data;
    };
    rb_color_t color = rb_red;

    ~rb_node() {
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
    rb_node(const rb_node&) = delete;
    rb_node(rb_node&&) = delete;
    rb_node& operator=(const rb_node&) = delete;
    rb_node& operator=(rb_node&&) = delete;

    // hide this to prevent default-constructed data from creating head nodes
    struct rb_head_tag_t {};
    rb_node(rb_head_tag_t _tag) : parent(this) { (void)_tag; }

    rb_node(T data) : data(std::move(data)) {}
    template <typename... Args>
    rb_node(Args&&... args) : data(std::forward<Args>(args)...) {}

  public:
    static node_t* new_empty() { return new rb_node(rb_head_tag_t{}); }

    template <typename... Args>
    static node_t* make(Args&&... args) {
        return new rb_node(std::forward<Args>(args)...);
    }
};

/**
 * Red-black binary search tree core
 * Tree is completely open
 */
template <typename T>
struct rb_tree {
    using node_t = rb_node<T>;

    // The real tree's root is head->link[0]. head is never nullptr.
    node_t* head;
    node_t* min_node;
    node_t* max_node;
    size_t node_count;

    rb_tree() noexcept
        : head(node_t::new_empty()), min_node(head), max_node(head), node_count(0) {}

    // Move constructor
    rb_tree(rb_tree&& other) noexcept : rb_tree() { swap(other); }
    // Copy constructor
    rb_tree(const rb_tree& other) noexcept
        : head(node_t::new_empty()), node_count(other.node_count) {
        adopt_node(head, deep_clone_node(other.head->link[0]), 0);
        update_minmax();
    }
    // Move assignment
    rb_tree& operator=(rb_tree&& other) noexcept {
        if (head != other.head) {
            clear();
            swap(other);
        }
        return *this;
    }
    // Copy assignment
    rb_tree& operator=(const rb_tree& other) noexcept {
        if (head != other.head) {
            delete head->link[0];
            adopt_node(head, deep_clone_node(other.head->link[0]), 0);
            update_minmax();
            node_count = other.node_count;
        }
        return *this;
    }

    ~rb_tree() noexcept { delete head; }

    void clear() noexcept {
        delete head->link[0];
        head->link[0] = nullptr;
        min_node = max_node = head;
        node_count = 0;
    }
    void swap(rb_tree& other) noexcept {
        std::swap(head, other.head);
        std::swap(min_node, other.min_node);
        std::swap(max_node, other.max_node);
        std::swap(node_count, other.node_count);
    }
    friend void swap(rb_tree& lhs, rb_tree& rhs) noexcept { lhs.swap(rhs); }

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
        node->parent = nullptr;
        node->color = rb_red;
    }
    static node_t* deep_clone_node(const node_t* node) {
        if (!node)
            return nullptr;
        node_t* clone = node_t::make(node->data);
        clone->color = node->color;
        adopt_node(clone, deep_clone_node(node->link[0]), 0);
        adopt_node(clone, deep_clone_node(node->link[1]), 1);
        return clone;
    }

    static bool is_black(const node_t* node) noexcept {
        return !node || node->color == rb_black;
    }
    static bool is_red(const node_t* node) noexcept {
        return node && node->color == rb_red;
    }
    static bool is_black_blob(const node_t* node) noexcept {
        return node && node->color == rb_black && is_black(node->link[0]) &&
               is_black(node->link[1]);
    }

  public:
    /**
     *       y                     x
     *      / \                   / \
     *    [a]  x        ->       y  [c]
     *        / \               / \
     *      [b] [c]           [a] [b]
     */
    void rotate_left(node_t* y) {
        node_t* x = y->link[1];
        bool yside = y == y->parent->link[1];
        adopt_node(y->parent, x, yside);
        adopt_node(y, x->link[0], 1);
        adopt_node(x, y, 0);
    }

    /**
     *         y                  x
     *        / \                / \
     *       x  [c]     ->     [a]  y
     *      / \                    / \
     *    [a] [b]                [b] [c]
     */
    void rotate_right(node_t* y) {
        node_t* x = y->link[0];
        bool yside = y == y->parent->link[1];
        adopt_node(y->parent, x, yside);
        adopt_node(y, x->link[1], 0);
        adopt_node(x, y, 1);
    }

    /**
     *      __g__             _(g)_              __g__               __y__
     *     /     \  0        /     \  0         /     \  0          /     \  0
     *   (p)     (u)   ->   p       u         (p)      u    ->    (p)     (g)
     * 0 / \ +1  / \     0 / \ +1  / \      0 / \ +1  / \       0 / \ 0   / \
     *  .. (y)  .. ..     .. (y)  .. ..      .. (y)  .. ..       .. ..   ..  u
     *         step 1: p=red, u=red                 step 2: u=black
     *  recolor p,u,g and continue on g     rotate around p and recolor y,g
     *
     *  step 2 case 1: double rotation      step 2 case 2: single rotation
     *      __g__             __y__              __g__               __p__
     *     /     \  0     0  /     \  0         /     \  0          /     \  0
     *   (p)      u   ->   (p)     (g)        (p)      u    ->    (y)     (g)
     * 0 / \ +1            / \     / \     +1 / \ 0             0 / \ 0   / \
     *  .. (y)            ..  a   b   u     (y)  b               .. ..   b   u
     *     / \
     *    a   b
     *
     *  whenever p is black we're done.
     */
    void rebalance_after_insert(node_t* y) {
        node_t* p = y->parent;
        node_t* g = p->parent;
        node_t* u = g->link[p != g->link[1]];
        assert(p && g);
        while (g != head && is_red(p) && is_red(u)) {
            p->color = rb_black, u->color = rb_black, g->color = rb_red;
            y = g, p = y->parent, g = p->parent, u = g->link[p != g->link[1]];
        }
        if (p == head)
            y->color = rb_black;
        else if (is_red(p)) {
            g->color = rb_red;
            bool yp = y == p->link[1];
            bool pg = p == g->link[1];

            if (yp && !pg)
                y->color = rb_black, rotate_left(p), rotate_right(g);
            else if (!yp && !pg)
                p->color = rb_black, rotate_right(g);
            else if (yp)
                p->color = rb_black, rotate_left(g);
            else
                y->color = rb_black, rotate_right(p), rotate_left(g);
        }
    }

    /**
     * there is -1 black under n that needs fixing.
     *
     *       p                p            .
     *  -1  / \  0        0  / \  0        step 1: n is red
     *     /   \     ->     /   \          just recolor n black and we're done
     *   (n)    s          n     s   done  otherwise, n is black.
     *
     *       p                p            .
     *  -1  / \  0       -1  / \  -1       step 2: p,s,l,r are black (and l,r exist)
     *     n   s     ->     n  (s)         not a whole lot can be done. recolor s red,
     *        / \              / \         so now the whole thing lacks blacks and we
     *       l   r            l   r  loop  recurse on p. this is a loop.
     *
     *       p                s            .
     *  -1  / \  0           / \  0        step 3: s is red
     *     n  (s)    ->    (p)  r          rotate left at p and recolor p red, s black.
     *        / \       -1 / \ 0           now we fall into the easier steps 4-6
     *       l   r        n   l            where p is red. relabel s=l.
     *
     *      (p)               p            .
     *  -1  / \  0        0  / \  0        step 4: p is red, s,l,r are black
     *     n   s     ->     n  (s)         we pushdown the red of p to s and
     *        / \              / \         we're done.
     *       l   r            l   r  done  .
     *
     *       p?               p?           .
     *  -1  / \  0       -1  / \  0        step 5: l is red
     *     n   s     ->     n   l          rotate right at s and recolor s red,
     *        / \              / \         l black. relabel s=l.
     *      (l)  r           [a] (s)       notice this doesn't change any black
     *      / \                  / \       heights on the right.
     *    [a] [b]              [b]  r      .
     *
     *       p?               s?           .
     *  -1  / \  0        0  / \  0        step 6: r is red (guaranteed)
     *     n   s     ->     p   r          rotate left at p, recolor s with p's color.
     *        / \          / \             done.
     *       l  (r)       n   l      done  .
     */
    void rebalance_after_erase(node_t* p, bool np) {
        node_t* n = p->link[np];
        node_t* s = p->link[!np];

        // step 1
        if (is_red(n)) {
            n->color = rb_black;
            return;
        }

        // step 2
        while (is_black(p) && is_black_blob(s)) {
            s->color = rb_red;
            np = p == p->parent->link[1];
            p = p->parent;
            s = p->link[!np];
        }
        if (p == head)
            return;

        // step 3
        if (is_red(s)) {
            p->color = rb_red;
            s->color = rb_black;
            np ? rotate_right(p) : rotate_left(p);
            s = p->link[!np];
        }

        // step 4
        if (is_red(p) && is_black_blob(s)) {
            s->color = rb_red;
            p->color = rb_black;
            return;
        }
        // step 5
        if (is_red(s->link[np])) {
            s->color = rb_red;
            s->link[np]->color = rb_black;
            np ? rotate_left(s) : rotate_right(s);
            s = p->link[!np];
        }
        // step 6
        s->color = p->color;
        p->color = rb_black;
        if (s->link[!np])
            s->link[!np]->color = rb_black;
        np ? rotate_right(p) : rotate_left(p);
    }

    /**
     *   parent   parent  <-- rebalance here
     *     |        |
     *     y   ->   x
     *    /
     * [(x)]
     */
    void erase_node_pull_left(node_t* y) {
        node_t* x = y->link[0];
        node_t* parent = y->parent;
        bool yside = y == parent->link[1];
        adopt_node(parent, x, yside);
        if (y->color == rb_black)
            rebalance_after_erase(parent, yside);
    }

    /**
     *     |            |
     *     y            x  <-- rebalance here
     *    / \    ->    / \
     *  [a]  x       [a] [b]
     *        \                visualize this as swapping x and y
     *        [b]              and then erasing y with inverse of pull_left
     */
    void erase_node_pull_right(node_t* y) {
        node_t* x = y->link[1];
        node_t* parent = y->parent;
        bool yside = y == parent->link[1];
        adopt_node(parent, x, yside);
        adopt_node(x, y->link[0], 0);
        std::swap(x->color, y->color);
        if (y->color == rb_black)
            rebalance_after_erase(x, 1);
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
        std::swap(x->color, y->color);
        if (y->color == rb_black)
            rebalance_after_erase(w, 0);
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
     *   [l]            [l]  (y)                    [r]      (y)  [r]
     */
    void insert_node(node_t* parent, node_t* y, bool side) {
        insert_minmax(parent, y, side);
        adopt_node(parent, y, side);
        assert(y->color == rb_red);
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
     *   [l]            [l]  (y)               [l]   r       [l]   r
     *                                              /             /
     *                                            ...           ...
     *                                            /             /
     *                              ++parent --> x             x
     *                                            \           / \
     *                                            [r]       (y) [r]
     */
    void insert_node_after(node_t* parent, node_t* y) {
        if (parent->link[1])
            insert_node(node_t::increment(parent), y, 0);
        else
            insert_node(parent, y, 1);
    }

    /**
     * Insert node y before node, so that decrementing node afterwards gives y.
     * Usually this will insert y as the left child of node.
     * y must be a free node.
     *
     *  parent          parent                    parent          parent
     *       \    ->    /  \          or        /  \    ->    /  \
     *       [r]      (y)  [r]                 l   [r]       l   [r]
     *                                          \             \
     *                                          ...           ...
     *                                            \             \
     *                                --parent --> x             x
     *                                            /             / \
     *                                          [l]           [l] (y)
     */
    void insert_node_before(node_t* parent, node_t* y) {
        if (parent->link[0])
            insert_node(node_t::decrement(parent), y, 1);
        else
            insert_node(parent, y, 0);
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
