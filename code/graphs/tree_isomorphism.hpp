#pragma once

#include "hash.hpp"
#include "struct/tensor.hpp"

struct unrooted_topology {
    static inline unordered_map<vector<int>, int, Hasher> subt_cache;
    static inline unordered_map<pair<int, int>, int, Hasher> cache;
    using Edges = vector<array<int, 2>>;
    using Tree = vector<basic_string<int>>;

    // * Auxiliary stuff...
    static auto get_nodes_in_tree(int V, int r, int p, const Tree& tree) {
        static thread_local int S = 0;
        static thread_local vector<int> parent;
        S = max(V, S), parent.resize(S);
        parent[r] = -1;
        vector<int> bfs = {r};
        for (int i = 0, B = 1; i < B; i++) {
            int u = bfs[i];
            for (int v : tree[u]) {
                if (v != parent[u] && v != p) {
                    bfs.push_back(v), B++;
                    parent[v] = u;
                }
            }
        }
        return bfs;
    }

    static auto get_reps_in_forest(int V, const Tree& tree) {
        static thread_local vector<int8_t> open;
        open.assign(V, 1);
        vector<int> reps;
        for (int u = 0; u < V; u++) {
            if (open[u]) {
                reps.push_back(u);
                for (int v : get_nodes_in_tree(V, u, -1, tree)) {
                    open[v] = false;
                }
            }
        }
        return reps;
    }

    static auto make_tree(int V, const Edges& edges) {
        Tree tree(V);
        for (auto [u, v] : edges) {
            tree[u].push_back(v);
            tree[v].push_back(u);
        }
        return tree;
    }

    static auto get_subt(const vector<int>& hashes) {
        if (auto pos = subt_cache.find(hashes); pos != end(subt_cache)) {
            return make_pair(pos->second, false);
        } else {
            int S = subt_cache.size();
            subt_cache[hashes] = S;
            return make_pair(S, true);
        }
    }

    static auto get_cache(int a, int b) {
        if (auto pos = cache.find({a, b}); pos != end(cache)) {
            return make_pair(pos->second, false);
        } else {
            int S = cache.size();
            cache[{a, b}] = cache[{b, a}] = S;
            return make_pair(S, true);
        }
    }

    static int hash_rooted(int V, int r, int p, const Tree& tree) {
        static thread_local int S = 0;
        static thread_local vector<vector<int>> hashes;
        static thread_local vector<int> bfs, parent;
        S = max(S, V), hashes.resize(S), bfs.resize(S), parent.resize(S);
        bfs[0] = r, parent[r] = -1;
        int B = 1;
        for (int i = 0; i < B && B < V; i++) {
            int u = bfs[i];
            for (int v : tree[u]) {
                if (v != parent[u] && v != p) {
                    bfs[B++] = v;
                    parent[v] = u;
                }
            }
        }
        for (int i = B - 1; i > 0; i--) {
            int u = bfs[i];
            sort(begin(hashes[u]), end(hashes[u]));
            int h = get_subt(hashes[u]).first;
            hashes[u].clear();
            hashes[parent[u]].push_back(h);
        }
        sort(begin(hashes[r]), end(hashes[r]));
        int h = get_subt(hashes[r]).first;
        hashes[r].clear();
        return h;
    }

    static auto hash_unrooted(int V, int r, int p, const Tree& tree) {
        static thread_local int S = 0;
        static thread_local vector<int> subt, ok, bfs, parent;
        S = max(S, V), subt.resize(S), ok.resize(S), bfs.resize(S), parent.resize(S);
        bfs[0] = r, parent[r] = -1, subt[r] = ok[r] = 1;
        int B = 1;
        for (int i = 0; i < B && B < V; i++) {
            int u = bfs[i];
            for (int v : tree[u]) {
                if (v != parent[u] && v != p) {
                    bfs[B++] = v;
                    parent[v] = u;
                    subt[v] = ok[v] = 1;
                }
            }
        }
        vector<int> ans;
        for (int i = B - 1; i >= 0 && ans.size() < 2u; i--) {
            int u = bfs[i];
            if (ok[u] && B - subt[u] <= B / 2) {
                ans.push_back(hash_rooted(V, u, p, tree));
            }
            if (parent[u] != -1) {
                subt[parent[u]] += subt[u];
                ok[parent[u]] &= subt[u] <= B / 2;
            }
        }
        int A = ans.size();
        assert(A == 1 || A == 2);
        return get_cache(ans[0], A == 2 ? ans[1] : -1);
    }

    // * Main interface...
    static void clear() { subt_cache.clear(), cache.clear(); }

    // Compute unrooted hash for given tree
    static int hash_tree(int V, const Edges& edges) {
        auto tree = make_tree(V, edges);
        return hash_unrooted(V, 0, -1, tree).first;
    }

    // Compute unrooted hash for given tree
    static int hash_tree(int V, int r, const Edges& edges) {
        auto tree = make_tree(V, edges);
        return hash_unrooted(V, r, -1, tree).first;
    }

    // Compute unrooted hash for each tree
    static auto hash_forest_by_node(int V, const Edges& edges) {
        auto tree = make_tree(V, edges);
        vector<int> hashes(V);
        for (int u = 0; u < V; u++) {
            hashes[u] = hash_unrooted(V, u, -1, tree).first;
        }
        return hashes;
    }

    // Compute set of hashes for forest
    static auto hash_forest(int V, const Edges& edges) {
        auto tree = make_tree(V, edges);
        vector<pair<int, int>> hashes;
        for (int u : get_reps_in_forest(V, tree)) {
            hashes.emplace_back(u, hash_unrooted(V, u, -1, tree).first);
        }
        return hashes;
    }

    // Compute the two unrooted hashes for given tree with edge e removed. O(V)
    static auto hash_removed_edge(int V, const Edges& edges, int e) {
        auto tree = make_tree(V, edges);
        auto [u, v] = edges[e];
        int a = hash_unrooted(V, u, v, tree).first;
        int b = hash_unrooted(V, v, u, tree).first;
        return make_pair(a, b);
    }

    // Compute the 2n-2 unrooted hashes for given tree with each edge removed. O(V^2)
    static auto hash_removed_edges(int V, const Edges& edges) {
        auto tree = make_tree(V, edges);
        vector<pair<int, int>> hashes;
        for (auto [u, v] : edges) {
            int a = hash_unrooted(V, u, v, tree).first;
            int b = hash_unrooted(V, v, u, tree).first;
            hashes.emplace_back(a, b);
        }
        return hashes;
    }

    // Compute the deg(n) unrooted hash sets for given tree with vertex removed. O(V deg)
    static auto hash_removed_vertex(int V, const Edges& edges, int u) {
        auto tree = make_tree(V, edges);
        vector<pair<int, int>> hashes;
        for (int v : tree[u]) {
            int a = hash_unrooted(V, v, u, tree).first;
            hashes.emplace_back(v, a);
        }
        return hashes;
    }

    // Compute the 2n-2 unrooted hash sets for given tree with each vertex removed. O(V^2)
    static auto hash_removed_vertices(int V, const Edges& edges) {
        auto tree = make_tree(V, edges);
        vector<vector<pair<int, int>>> hashes(V);
        for (int u = 0; u < V; u++) {
            for (int v : tree[u]) {
                int a = hash_unrooted(V, v, u, tree).first;
                hashes[u].emplace_back(v, a);
            }
        }
        return hashes;
    }

    // Dfs postorder on all isomorphically distinct subtrees of the given tree
    // Only visits new hashes. Signature: (V, hash, edges[], tree[][])
    // Number of trees is O(2^(2V/3)) worst-case
    template <typename Fn>
    static bool visit_subtrees(int V, const Edges& edges, Fn&& visitor) {
        auto tree = make_tree(V, edges);
        auto [h, ok] = hash_unrooted(V, 0, -1, tree);
        if (!ok) {
            return false;
        }
        vector<int8_t> checked(V);
        for (auto [u, v] : edges) {
            if (tree[u].size() >= 2u && tree[v].size() >= 2u) {
                continue;
            }
            int p = tree[v].size() == 1u ? v : u;
            if (checked[u] || checked[v]) {
                continue;
            }
            checked[u] = checked[v] = true;
            Edges subt;
            for (auto [x, y] : edges) {
                if (x != p && y != p) {
                    subt.push_back({x == V - 1 ? p : x, y == V - 1 ? p : y});
                }
            }
            visit_subtrees(V - 1, subt, visitor);
        }
        visitor(V, h, edges, tree);
        return true;
    }
};

struct rooted_topology {
    static inline unordered_map<vector<int>, int, Hasher> cache;
    using Edges = vector<array<int, 2>>;
    using Tree = vector<basic_string<int>>;

    // * Auxiliary stuff...
    static auto make_tree(int V, const Edges& edges) {
        vector<basic_string<int>> tree(V);
        for (auto [u, v] : edges) {
            tree[u].push_back(v);
            tree[v].push_back(u);
        }
        return tree;
    }

    static auto make_subtree(int V, int r, int p, const Tree& tree) {
        static thread_local int S = 0;
        static thread_local vector<int> bfs, parent;
        S = max(S, V), bfs.resize(S), parent.resize(S);
        bfs[0] = r, parent[r] = -1;
        Edges ans;
        int B = 1;
        for (int i = 0; i < B && B < V; i++) {
            int u = bfs[i];
            for (int v : tree[u]) {
                if (v != p && v != parent[u]) {
                    ans.push_back({i, B});
                    bfs[B++] = v;
                    parent[v] = u;
                }
            }
        }
        return make_pair(B, move(ans));
    }

    static auto get_cache(const vector<int>& hashes) {
        if (auto pos = cache.find(hashes); pos != end(cache)) {
            return make_pair(pos->second, false);
        } else {
            int S = cache.size();
            cache[hashes] = S;
            return make_pair(S, true);
        }
    }

    static int hash_rooted(int V, int r, int p, const Tree& tree) {
        static thread_local int S = 0;
        static thread_local vector<vector<int>> hashes;
        static thread_local vector<int> bfs, parent;
        S = max(S, V), hashes.resize(S), bfs.resize(S), parent.resize(S);
        bfs[0] = r, parent[r] = -1;
        int B = 1;
        for (int i = 0; i < B && B < V; i++) {
            int u = bfs[i];
            for (int v : tree[u]) {
                if (v != parent[u] && v != p) {
                    bfs[B++] = v;
                    parent[v] = u;
                }
            }
        }
        for (int i = B - 1; i > 0; i--) {
            int u = bfs[i];
            sort(begin(hashes[u]), end(hashes[u]));
            int h = get_cache(hashes[u]).first;
            hashes[u].clear();
            hashes[parent[u]].push_back(h);
        }
        sort(begin(hashes[r]), end(hashes[r]));
        int h = get_cache(hashes[r]).first;
        hashes[r].clear();
        return h;
    }

    template <typename Fn>
    static void leaf_search(int V, const Edges& edges, const Tree& tree, Fn&& visitor) {
        vector<int8_t> checked(V);
        for (auto [u, v] : edges) {
            if (tree[u].size() >= 2u && tree[v].size() >= 2u) {
                continue;
            }
            int a = tree[v].size() == 1u ? u : v;
            int p = tree[v].size() == 1u ? v : u;
            if (checked[u] || checked[v] || p == 0) {
                continue;
            }
            checked[u] = checked[v] = true;
            Edges subt;
            for (auto [x, y] : edges) {
                if (x != p && y != p) {
                    subt.push_back({x == V - 1 ? p : x, y == V - 1 ? p : y});
                }
            }
            visit_subtrees(V - 1, 0, subt, visitor);
        }
    }

    // * Main interface...
    static void clear() { cache.clear(); }

    // Compute rooted hash for given tree. O(V)
    static int hash_tree(int V, int r, const Edges& edges) {
        auto tree = make_tree(V, edges);
        return hash_rooted(V, r, -1, tree);
    }

    // Compute rooted hash for given tree for every root. O(V^2)
    static auto hash_tree_all_roots(int V, const Edges& edges) {
        auto tree = make_tree(V, edges);
        vector<int> hashes;
        for (int r = 0; r < V; r++) {
            int a = hash_rooted(V, r, -1, tree);
            hashes.push_back(a);
        }
        return hashes;
    }

    // Compute rooted hash for given tree with subtree under u removed for every u. O(V^2)
    static auto hash_tree_removed(int V, int r, const Edges& edges) {
        auto tree = make_tree(V, edges);
        vector<int> hashes;
        for (int u = 0; u < V; u++) {
            int a = r == u ? get_cache({}).first : hash_rooted(V, r, u, tree);
            hashes.push_back(a);
        }
        return hashes;
    }

    // Dfs postorder on all isomorphically distinct rooted subtrees of the given tree
    // Only visits new hashes. Signature: (V, h, edges[], tree[][]), root is 0
    // Number of trees is O(2^(2V/3)) worst-case
    template <typename Fn>
    static void visit_subtrees(int V, int r, const Edges& edges, Fn&& visitor) {
        auto tree = make_tree(V, edges);
        vector<int> bfs(V), parent(V);
        bfs[0] = r, parent[r] = -1;
        int B = 1;
        for (int i = 0; i < B && B < V; i++) {
            int u = bfs[i];
            for (int v : tree[u]) {
                if (v != parent[u]) {
                    bfs[B++] = v;
                    parent[v] = u;
                }
            }
        }
        vector<vector<int>> hashes(V);
        for (int i = B - 1; i >= 0; i--) {
            int u = bfs[i];
            sort(begin(hashes[u]), end(hashes[u]));
            auto [h, ok] = get_cache(hashes[u]);
            if (ok) {
                auto [S, subt_edges] = make_subtree(V, u, parent[u], tree);
                auto subt = make_tree(S, subt_edges);
                leaf_search(S, subt_edges, subt, visitor);
                visitor(S, h, subt_edges, subt);
            }
            if (parent[u] != -1) {
                hashes[parent[u]].push_back(h);
            }
        }
    }
};

// Generate all unrooted binary trees with at most N internal nodes (degree 1 or 3)
// Each such tree has n internal nodes and n+2 leaves for some n
auto build_unrooted_binary_trees(int N) {
    using Edges = vector<array<int, 2>>;
    vector<vector<Edges>> ans(N + 1);
    ans[0].push_back({Edges{{0, 1}}});
    for (int k = 1, V = 4; k <= N; V += 2, k++) {
        unordered_set<int> seen;
        for (const Edges& G : ans[k - 1]) {
            // [0...k-1) are internal nodes and [k-1...2k) are leaf nodes
            // i i i l l x l l -> x i i i l l l l a b
            for (int x = k - 1; x < 2 * k; x++) {
                Edges H;
                for (auto [u, v] : G) {
                    int a = u != x ? u + (u < x) : 0;
                    int b = v != x ? v + (v < x) : 0;
                    H.push_back({a, b});
                }
                H.push_back({0, V - 2});
                H.push_back({0, V - 1});
                int h = unrooted_topology::hash_tree(V, H);
                if (seen.insert(h).second) {
                    ans[k].push_back(move(H));
                }
            }
        }
    }
    return ans;
}

// Generate rooted, mostly distinct fork trees with at most N nodes and B branches
// Fork tree: center node 0 with a bunch of chains or 3-branch trees attached to it
auto build_branches(int N, int B, bool chains = true, bool forks = true) {
    using Edges = vector<array<int, 2>>;
    tensor<vector<Edges>, 3> ans({N + 1, B + 1, N + 1});
    ans[{1, 0, 0}].push_back({});
    for (int V = 2; V <= N && chains; V++) {
        Edges G;
        for (int i = 0; i < V - 1; i++) {
            G.push_back({i, i + 1});
        }
        ans[{V, 1, V - 1}].push_back(move(G));
    }
    for (int V = 2; V <= N && forks; V++) {
        for (int a = 1; a <= V - 3; a++) {
            for (int b = 1, c = V - a - 2; b <= c; b++, c--) {
                Edges G;
                for (int i = 0; i < a; i++) {
                    G.push_back({i, i + 1});
                }
                for (int i = 0; i < b; i++) {
                    G.push_back({i == 0 ? a : a + i, a + i + 1});
                }
                for (int i = 0; i < c; i++) {
                    G.push_back({i == 0 ? a : a + b + i, a + b + i + 1});
                }
                ans[{V, 1, V - 1}].push_back(move(G));
            }
        }
    }
    for (int b = 2; b <= B; b++) {
        for (int V = b + 1; V <= N; V++) {
            for (int a = 1; a < V; a++) {              // length of largest previous fork
                for (int c = a; 1 + a + c <= V; c++) { // length of the next largest fork
                    int S = V - c;
                    for (const Edges& G : ans[{S, b - 1, a}]) {
                        for (const Edges& H : ans[{c + 1, 1, c}]) {
                            auto T = G;
                            for (auto [u, v] : H) {
                                if (u != 0) {
                                    T.push_back({u + S - 1, v + S - 1});
                                }
                            }
                            T.push_back({0, S});
                            ans[{V, b, c}].push_back(move(T));
                        }
                    }
                }
            }
        }
    }
    return ans;
}
