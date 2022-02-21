#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "avl_tree.hpp"
#include "rb_tree.hpp"
#include "splay_tree.hpp"

using fmt::print, fmt::format;

template <typename Tree>
struct tree_debugger {
    const Tree& t;

    explicit tree_debugger(const Tree& tree) : t(tree) {}

    void pretty_print() const {
        print("======== count: {:02} ========\n", int(t.node_count));
        print_tree_preorder(t.head->link[0], "");
        print("===========================\n");
    }

    void debug() const {
        const auto& tree = t.internal();
        debug_tree(tree), verify_node(t.head->link[0], t.head, t.comp);
    }

  private:
    template <typename Node>
    static void print_tree_preorder(const Node* n, std::string prefix, bool bar = false) {
        static const char* line[2] = {u8"└──", u8"├──"};
        static const char* pad[2] = {"    ", u8" |  "};
        if (!n) {
            return print("{} {}\n", prefix, line[bar]);
        }
        print("{} {} {}\n", prefix, line[bar], print_node(n));
        if (n->link[0] || n->link[1]) {
            prefix += pad[bar];
            print_tree_preorder(n->link[0], prefix, true);
            print_tree_preorder(n->link[1], prefix, false);
        }
    }

    // BS Tree

    template <typename Node, typename Compare>
    static void verify_node(const Node* y, const Node* parent, const Compare& comp) {
        if (y && parent != parent->parent) {
            assert(y != parent->link[0] || !comp(parent->data, y->data));
            assert(y != parent->link[1] || !comp(y->data, parent->data));
        }
        if (y) {
            verify_node(y->link[0], y, comp);
            verify_node(y->link[1], y, comp);
        }
    }

    // AVL Tree

    template <typename T>
    static auto print_node(const avl_node<T>* node) noexcept {
        using std::to_string;
        std::string s;
        s += to_string(node->data);
        s += "(" + to_string(node->balance) + ")";
        s += u8"  ╴  ╴  ╴  ╴ ";
        if (node->parent != node->parent->parent)
            s += "  ^(" + to_string(node->parent->data) + ")";
        if (node->link[0])
            s += "  <(" + to_string(node->link[0]->data) + ")";
        if (node->link[1])
            s += "  >(" + to_string(node->link[1]->data) + ")";
        return s;
    }

    template <typename T>
    static int debug_node(const avl_node<T>* y, const avl_node<T>* parent, size_t& cnt) {
        if (!y)
            return 0;
        cnt++;
        assert(y->parent == parent);
        assert(-1 <= y->balance && y->balance <= +1);
        int l = debug_node(y->link[0], y, cnt);
        int r = debug_node(y->link[1], y, cnt);
        assert(y->balance == r - l);
        return 1 + std::max(l, r);
    }

    template <typename T>
    static void debug_tree(const avl_tree<T>& t) {
        assert(t.head && !t.head->link[1] && t.head->balance == 0);
        assert(t.head->parent == t.head);
        if (t.head->link[0]) {
            assert(t.min_node == avl_node<T>::minimum(t.head->link[0]));
            assert(t.max_node == avl_node<T>::maximum(t.head->link[0]));
        } else {
            assert(t.min_node == t.head && t.max_node == t.head);
        }
        size_t cnt = 0;
        debug_node(t.head->link[0], t.head, cnt);
        assert(cnt == t.node_count);
    }

    // Red Black Tree

    template <typename T>
    static auto print_node(const rb_node<T>* node) noexcept {
        using std::to_string;
        std::string s;
        s += to_string(node->data);
        if (node->color == rb_red)
            s += "(**)";
        s += u8"  ╴  ╴  ╴  ╴ ";
        if (node->parent != node->parent->parent)
            s += "  ^(" + to_string(node->parent->data) + ")";
        if (node->link[0])
            s += "  <(" + to_string(node->link[0]->data) + ")";
        if (node->link[1])
            s += "  >(" + to_string(node->link[1]->data) + ")";
        return s;
    }

    template <typename T>
    static int debug_node(const rb_node<T>* y, const rb_node<T>* parent, size_t& cnt) {
        if (!y)
            return 0;
        cnt++;
        (void)parent, assert(y->parent == parent);
        assert(parent->color == rb_black || y->color == rb_black);
        int bhl = debug_node(y->link[0], y, cnt);
        int bhr = debug_node(y->link[1], y, cnt);
        (void)bhr, assert(bhl == bhr);
        return bhl + y->color;
    }

    template <typename T>
    static void debug_tree(const rb_tree<T>& t) {
        assert(t.head && !t.head->link[1] && t.head->color == rb_red);
        assert(t.head->parent == t.head);
        if (t.head->link[0]) {
            assert(t.min_node == rb_node<T>::minimum(t.head->link[0]));
            assert(t.max_node == rb_node<T>::maximum(t.head->link[0]));
        } else {
            assert(t.min_node == t.head && t.max_node == t.head);
        }
        size_t cnt = 0;
        debug_node(t.head->link[0], t.head, cnt);
        assert(cnt == t.node_count);
    }

    // Splay Tree

    template <typename T>
    static auto print_node(const splay_node<T>* node) noexcept {
        using std::to_string;
        std::string s;
        s += to_string(node->data);
        s += u8"  ╴  ╴  ╴  ╴ ";
        if (node->parent != node->parent->parent)
            s += "  ^(" + to_string(node->parent->data) + ")";
        if (node->link[0])
            s += "  <(" + to_string(node->link[0]->data) + ")";
        if (node->link[1])
            s += "  >(" + to_string(node->link[1]->data) + ")";
        return s;
    }

    template <typename T>
    static int debug_node(const splay_node<T>* y, const splay_node<T>* parent,
                          size_t& cnt) {
        if (!y)
            return 0;
        cnt++;
        (void)parent, assert(y->parent == parent);
        int hl = debug_node(y->link[0], y, cnt);
        int hr = debug_node(y->link[1], y, cnt);
        return 1 + std::max(hl, hr);
    }

    template <typename T>
    static void debug_tree(const splay_tree<T>& t) {
        assert(t.head && !t.head->link[1] && t.head->parent == t.head);
        if (t.head->link[0]) {
            assert(t.min_node == splay_node<T>::minimum(t.head->link[0]));
            assert(t.max_node == splay_node<T>::maximum(t.head->link[0]));
        } else {
            assert(t.min_node == t.head && t.max_node == t.head);
        }
        size_t cnt = 0;
        debug_node(t.head->link[0], t.head, cnt);
        assert(cnt <= t.node_count);
        assert(cnt >= t.node_count);
    }
};

template <typename BSTree>
auto debug_tree(const BSTree& tree) {
    return tree_debugger(tree);
}
