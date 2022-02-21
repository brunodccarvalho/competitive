#pragma once

#include "lib/graph_formats.hpp"
#include "random.hpp"
#include "struct/pbds.hpp"

/**
 * An implementation of common link-cut tree mechanics using trivial data structures,
 * along with helpful utilities.
 */
template <bool rooted>
struct slow_tree {
    // global forest topology
    using tree_pt = list<ordered_set<int>>::iterator;
    int V;
    ordered_set<pair<int, int>> edges; // list of edges
    ordered_set<int> roots, non_roots; // list of roots and non-roots nodes
    list<ordered_set<int>> trees;      // composition of trees
    vector<tree_pt> tree_of;           // tree of each node

    // node data/topology
    vector<long> val;                  // int value at each node
    vector<int> parent;                // parent of each node (0 for roots)
    vector<ordered_set<int>> children; // children list of each node
    vector<ordered_set<int>> adjacent; // children + parent

    // helpers for bfs
    mutable vector<uint> vis;
    mutable vector<int> bfs;
    mutable uint bfs_timer = 1;

    static inline boold coind = boold(0.5);

    explicit slow_tree(int V)
        : V(V), tree_of(V + 1), val(V + 1), parent(V + 1, 0), children(V + 1),
          adjacent(V + 1), vis(V + 1), bfs(V + 1) {
        for (int u = 1; u <= V; u++) {
            ordered_set<int> nums;
            nums.insert(u);
            tree_of[u] = trees.insert(end(trees), move(nums));
            roots.insert(u);
        }
    }

    int num_nodes() const { return V; }
    int num_edges() const { return edges.size(); }
    int num_trees() const { return trees.size(); }
    bool has_edge(int u, int v) const { return edges.find(minmax(u, v)) != end(edges); }

    // ***** TREE OPERATIONS

    int bfs_subtree(int u) const {
        int i = 0, S = 1;
        bfs[0] = u, vis[u] = ++bfs_timer;
        while (i < S) {
            for (int v : children[bfs[i++]]) {
                if (vis[v] < bfs_timer) {
                    bfs[S++] = v;
                    vis[v] = bfs_timer;
                }
            }
        }
        return S;
    }

    int bfs_tree(int u) const {
        int i = 0, S = 1;
        bfs[0] = u, vis[u] = ++bfs_timer;
        while (i < S) {
            for (int v : adjacent[bfs[i++]]) {
                if (vis[v] < bfs_timer) {
                    bfs[S++] = v;
                    vis[v] = bfs_timer;
                }
            }
        }
        return S;
    }

    void merge_trees(int u, int v) {
        assert(u != v && tree_of[u] != tree_of[v]);
        if (tree_of[u]->size() < tree_of[v]->size()) {
            swap(u, v);
        }
        auto old = tree_of[v];
        for (int w : *tree_of[v]) {
            tree_of[w] = tree_of[u];
            tree_of[u]->insert(w);
        }
        trees.erase(old);
    }

    void split_trees(int u, int v) {
        assert(u != v && tree_of[u] == tree_of[v]);
        if (coind(mt)) {
            swap(u, v);
        }
        int S = bfs_tree(u);
        ordered_set<int> u_tree(begin(bfs), begin(bfs) + S);
        auto new_tree = trees.insert(end(trees), move(u_tree));

        for (int i = 0; i < S; i++) {
            tree_of[bfs[i]]->erase(bfs[i]);
            tree_of[bfs[i]] = new_tree;
        }
    }

    tree_pt random_tree() const { return tree_of[random_node()]; }

    int random_in_tree(tree_pt tree) const {
        int S = tree->size();
        return *tree->find_by_order(intd(0, S - 1)(mt));
    }

    tree_pt different_tree(tree_pt tree) const {
        int R = roots.size();
        int i = intd(0, R - 1)(mt);
        int r = *roots.find_by_order(i);
        if (tree_of[r] == tree) {
            return tree_of[*roots.find_by_order(different<int>(i, 0, R - 1))];
        } else {
            return tree_of[r];
        }
    }

    // ***** UTILS

    int random_node() const { return intd(1, V)(mt); }

    int random_in_tree(int u) const { return random_in_tree(tree_of[u]); }

    auto random_edge() const {
        int E = edges.size();
        assert(E > 0);
        return *edges.find_by_order(intd(0, E - 1)(mt));
    }

    int random_root() const {
        int R = roots.size();
        assert(R > 0);
        return *roots.find_by_order(intd(0, R - 1)(mt));
    }

    int random_non_root() const {
        int R = non_roots.size();
        assert(R > 0);
        return *non_roots.find_by_order(intd(0, R - 1)(mt));
    }

    int random_adjacent(int u) const {
        int A = adjacent[u].size();
        assert(A > 0);
        return *adjacent[u].find_by_order(intd(0, A - 1)(mt));
    }

    int random_child(int u) const {
        int A = children[u].size();
        assert(A > 0);
        return *children[u].find_by_order(intd(0, A - 1)(mt));
    }

    auto random_connected() const {
        auto tree = tree_of[random_node()];
        int A = tree->size();
        int a = intd(0, A - 1)(mt), b = intd(0, A - 1)(mt);
        return make_pair(*tree->find_by_order(a), *tree->find_by_order(b));
    }

    auto random_connected_distinct() const {
        auto tree = tree_of[random_non_root()];
        int A = tree->size();
        assert(A > 1);
        auto [a, b] = different<int>(0, A - 1);
        return make_pair(*tree->find_by_order(a), *tree->find_by_order(b));
    }

    auto random_unconnected() const {
        int R = roots.size();
        assert(R > 1);
        auto [i, j] = different<int>(0, R - 1);
        int u = *roots.find_by_order(i), v = *roots.find_by_order(j);
        auto [a, b] = make_pair(random_in_tree(tree_of[u]), random_in_tree(tree_of[v]));
        if (tree_of[a] != tree_of[u] || tree_of[b] != tree_of[v]) {
            cout << "BUG" << endl;
        }
        return make_pair(a, b);
    }

    int random_unconnected(int u) const {
        assert(num_trees() > 1);
        return random_in_tree(different_tree(tree_of[u]));
    }

    int random_distinct_in_tree(int u) const {
        assert(tree_of[u]->size() > 1u);
        int bad = tree_of[u]->order_of_key(u);
        int j = different<int>(bad, 0, tree_of[u]->size() - 1);
        int v = *tree_of[u]->find_by_order(j);
        assert(u != v && tree_of[u] == tree_of[v]);
        return v;
    }

    int random_in_subtree(int u) const {
        int S = bfs_subtree(u);
        return bfs[intd(0, S - 1)(mt)];
    }

    int random_distinct_in_subtree(int u) const {
        int S = bfs_subtree(u);
        assert(S > 1);
        return bfs[intd(1, S - 1)(mt)];
    }

    int random_ancestor(int u) const {
        int S = get_root_path(u);
        return bfs[intd(0, S - 1)(mt)];
    }

    int random_distinct_ancestor(int u) const {
        assert(parent[u] != 0);
        int S = get_root_path(parent[u]);
        return bfs[intd(0, S - 1)(mt)];
    }

    // ***** GETTERS

    int get_root_path(int u) const {
        int S = 0;
        do {
            bfs[S++] = u, u = parent[u];
        } while (u);
        return S;
    }

    vector<int> get_path(int u, int v) const {
        int c = lca(u, v);
        vector<int> a = {u}, b = {v};
        while (u != c && u) {
            a.push_back(u = parent[u]);
        }
        while (v != c && v) {
            b.push_back(v = parent[v]);
        }
        assert(a.back() == b.back());
        b.pop_back();
        a.insert(end(a), rbegin(b), rend(b));
        return a;
    }

    // ***** REROOTING

    void flip_to_child(int u, int c) {
        if (parent[u]) {
            flip_to_child(parent[u], u);
        } else {
            roots.erase(u), non_roots.insert(u);
        }
        parent[u] = c;
        children[c].insert(u);
        children[u].erase(c);
    }

    // ***** CORE

    void link(int u, int v) {
        assert(u != v);
        if constexpr (!rooted) {
            assert(tree_of[u] != tree_of[v]);
            reroot(u);
        } else if constexpr (rooted) {
            if (parent[u] != 0) {
                cut(u);
            }
        }
        parent[u] = v;
        children[v].insert(u);
        adjacent[u].insert(v);
        adjacent[v].insert(u);
        edges.insert(minmax(u, v));
        roots.erase(u);
        non_roots.insert(u);
        merge_trees(u, v);
    }

    void cut(int u) {
        static_assert(rooted);
        assert(parent[u] != 0 && has_edge(u, parent[u]));
        int v = parent[u];
        parent[u] = 0;
        children[v].erase(u);
        adjacent[u].erase(v);
        adjacent[v].erase(u);
        edges.erase(minmax(u, v));
        non_roots.erase(u);
        roots.insert(u);
        split_trees(u, v);
    }

    void cut(int u, int v) {
        static_assert(!rooted);
        assert(u != v && has_edge(u, v));
        reroot(v);
        assert(parent[u] == v);
        parent[u] = 0;
        children[v].erase(u);
        adjacent[u].erase(v);
        adjacent[v].erase(u);
        edges.erase(minmax(u, v));
        non_roots.erase(u);
        roots.insert(u);
        split_trees(u, v);
    }

    void reroot(int u) {
        if (u && parent[u]) {
            flip_to_child(parent[u], u);
            parent[u] = 0;
            roots.insert(u);
            non_roots.erase(u);
        }
    }

    int lca(int u, int v) const {
        int a = u, b = v, ra = 0, rb = 0;
        while (a != b) {
            a = parent[a] ? parent[a] : (ra = a, v);
            b = parent[b] ? parent[b] : (rb = b, u);
            if (ra != 0 && rb != 0 && ra != rb)
                return 0; // different trees
        }
        return a;
    }

    int findroot(int u) const {
        while (parent[u]) {
            u = parent[u];
        }
        return u;
    }

    bool conn(int u, int v) const { return tree_of[u] == tree_of[v]; }

    // ***** NODE QUERIES

    auto query_node(int u) const { return val[u]; }

    void update_node(int u, long new_value) { val[u] = new_value; }

    // ***** TREES

    auto query_tree(int u) {
        long sum = 0;
        for (int w : *tree_of[u]) {
            sum += val[w];
        }
        return sum;
    }

    int tree_size(int u) { return tree_of[u]->size(); }

    void update_tree(int u, long value) {
        for (int w : *tree_of[u]) {
            val[w] += value;
        }
    }

    // ***** ROOTED SUBTREES

    auto query_subtree(int u) {
        static_assert(rooted);
        long sum = 0;
        for (int i = 0, S = bfs_subtree(u); i < S; i++) {
            sum += val[bfs[i]];
        }
        return sum;
    }

    int subtree_size(int u) {
        static_assert(rooted);
        return bfs_subtree(u);
    }

    void update_subtree(int u, long value) {
        static_assert(rooted);
        for (int i = 0, S = bfs_subtree(u); i < S; i++) {
            val[bfs[i]] += value;
        }
    }

    // ***** UNROOTED SUBTREES

    auto query_subtree(int u, int v) {
        static_assert(!rooted);
        reroot(v);
        long sum = 0;
        for (int i = 0, S = bfs_subtree(u); i < S; i++) {
            sum += val[bfs[i]];
        }
        return sum;
    }

    int subtree_size(int u, int v) {
        static_assert(!rooted);
        reroot(v);
        return bfs_subtree(u);
    }

    void update_subtree(int u, int v, long value) {
        static_assert(!rooted);
        reroot(v);
        for (int i = 0, S = bfs_subtree(u); i < S; i++) {
            val[bfs[i]] += value;
        }
    }

    // ***** PATHS

    auto query_path(int u, int v) {
        long sum = 0;
        for (int w : get_path(u, v)) {
            sum += val[w];
        }
        return sum;
    }

    int path_length(int u, int v) { return get_path(u, v).size(); }

    void update_path(int u, int v, long value) {
        for (int w : get_path(u, v)) {
            val[w] += value;
        }
    }
};

template <bool rooted>
void show_slow(const slow_tree<rooted>& st) {
    const auto spaced = [&](string label, const auto& container) {
        string s = format("{:10} ", label);
        int i = -1;
        for (const auto& n : container)
            if (++i >= 1)
                s += format(" {}={}", i, n);
        return s + "\n";
    };
    print("TREES: {}\n", st.trees);
    print("ROOTS: {}\n", st.roots);
    print(" SUBTREES:\n");
    for (int r : st.roots) {
        print("   {}: {}\n", r, *st.tree_of[r]);
    }
    print(spaced("parent", st.parent));
    print(spaced("adjacent", st.adjacent));
    print(spaced("children", st.children));
    print("\n");
}
