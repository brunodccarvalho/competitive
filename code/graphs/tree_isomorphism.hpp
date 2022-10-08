#pragma once

#include "hash.hpp"
#include "struct/tensor.hpp"

struct unrooted_topology {
    static inline unordered_map<vector<int>, int, Hasher> cache;
    static inline unordered_set<int> visited_topologies;
    using Edges = vector<array<int, 2>>;
    using Tree = vector<basic_string<int>>;

    static void clear() { cache.clear(), visited_topologies.clear(); }

    // * Auxiliary stuff...

    static auto make_tree(int V, const Edges& edges) {
        Tree tree(V);
        for (auto [u, v] : edges) {
            tree[u].push_back(v);
            tree[v].push_back(u);
        }
        return tree;
    }

    static auto get(const vector<int>& hashes) {
        if (auto pos = cache.find(hashes); pos != end(cache)) {
            return pos->second;
        } else {
            int S = cache.size();
            return cache[hashes] = S;
        }
    }

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

    // Compute rooted hash for r's tree and not passing through the edge towards p
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
            int h = get(hashes[u]);
            hashes[u].clear();
            hashes[parent[u]].push_back(h);
        }
        sort(begin(hashes[r]), end(hashes[r]));
        int h = get(hashes[r]);
        hashes[r].clear();
        return h;
    }

    // Compute (unrooted hash, centroid), prefering centroid with largest rooted hash
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
        auto ans = make_pair(-1, -1);
        int A = 0;
        for (int i = B - 1; i >= 0 && A < 2; i--) {
            int u = bfs[i];
            if (ok[u] && B - subt[u] <= B / 2) {
                ans = max(ans, make_pair(hash_rooted(V, u, p, tree), u)), A++;
            }
            if (parent[u] != -1) {
                subt[parent[u]] += subt[u];
                ok[parent[u]] &= subt[u] <= B / 2;
            }
        }
        return ans;
    }

    // For every node u compute ordered set of (h,v) among u's kids and populate kids[]
    static auto dfs_hashing(int V, int r, int p, const Tree& tree,
                            vector<vector<pair<int, int>>>& kids) {
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
            sort(begin(kids[u]), end(kids[u]));
            int h = get(hashes[u]);
            hashes[u].clear();
            kids[parent[u]].emplace_back(h, u);
            hashes[parent[u]].push_back(h);
        }
        sort(begin(hashes[r]), end(hashes[r]));
        sort(begin(kids[r]), end(kids[r]));
        int h = get(hashes[r]);
        hashes[r].clear();
        return h;
    }

    // Run dfs and use akids[] and bkids[] to match nodes between A and B with same hash
    static void dfs_matching(int V, int a, int b,
                             const vector<vector<pair<int, int>>>& akids,
                             const vector<vector<pair<int, int>>>& bkids,
                             vector<int>& atob) {
        atob[a] = b;
        assert(akids[a].size() == bkids[b].size());
        for (int i = 0, S = akids[a].size(); i < S; i++) {
            dfs_matching(V, akids[a][i].second, bkids[b][i].second, akids, bkids, atob);
        }
    }

    // * Main interface...

    static int hash_tree(int V, const Edges& edges) {
        auto tree = make_tree(V, edges);
        return hash_unrooted(V, 0, -1, tree).first;
    }

    static int hash_tree_in_forest(int V, int r, const Edges& edges) {
        auto tree = make_tree(V, edges);
        return hash_unrooted(V, r, -1, tree).first;
    }

    static auto hash_forest_by_node(int V, const Edges& edges) {
        auto tree = make_tree(V, edges);
        vector<int> hashes(V);
        for (int u = 0; u < V; u++) {
            hashes[u] = hash_unrooted(V, u, -1, tree).first;
        }
        return hashes;
    }

    // The hash of a forest is the sorted vector of hashes of each of its trees
    static auto hash_forest(int V, const Edges& edges) {
        static thread_local vector<int8_t> open;
        open.assign(V, 1);
        auto tree = make_tree(V, edges);
        vector<int> hashes;
        for (int u = 0; u < V; u++) {
            if (open[u]) {
                hashes.push_back(hash_unrooted(V, u, -1, tree).first);
                for (int v : get_nodes_in_tree(V, u, -1, tree)) {
                    open[v] = false;
                }
            }
        }
        sort(begin(hashes), end(hashes));
        return hashes;
    }

    // Compute vector of (tree hash,tree centroid) with the primary centroid on each tree
    static auto compute_forest_hashsets(int V, const Edges& edges) {
        static thread_local vector<int8_t> open;
        open.assign(V, 1);
        auto tree = make_tree(V, edges);
        vector<pair<int, int>> hashes;
        for (int u = 0; u < V; u++) {
            if (open[u]) {
                hashes.push_back(hash_unrooted(V, u, -1, tree));
                for (int v : get_nodes_in_tree(V, u, -1, tree)) {
                    open[v] = false;
                }
            }
        }
        sort(begin(hashes), end(hashes));
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

    // Compute the 2E unrooted hashes for given trees with each edge removed. O(V^2)
    static auto hash_removed_each_edge(int V, const Edges& edges) {
        auto tree = make_tree(V, edges);
        vector<pair<int, int>> hashes;
        for (auto [u, v] : edges) {
            int a = hash_unrooted(V, u, v, tree).first;
            int b = hash_unrooted(V, v, u, tree).first;
            hashes.emplace_back(a, b);
        }
        return hashes;
    }

    // Compute the deg(n) unrooted hash sets for given tree with u removed. O(V deg)
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
    static auto hash_removed_each_vertex(int V, const Edges& edges) {
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

    // If A and B are isomorphic forests return mapping of nodes from A to B, else nothing
    static auto isomorphic_mapping(int V, const Edges& A, const Edges& B) {
        auto areps = compute_forest_hashsets(V, A);
        auto breps = compute_forest_hashsets(V, B);
        if (areps.size() != breps.size()) {
            return vector<int>();
        }
        int T = areps.size();
        for (int i = 0; i < T; i++) {
            if (areps[i].first != breps[i].first) {
                return vector<int>();
            }
        }
        auto atree = make_tree(V, A);
        auto btree = make_tree(V, B);
        vector<vector<pair<int, int>>> akids(V), bkids(V);
        vector<int> ans(V, -1);
        for (int i = 0; i < T; i++) {
            int a = areps[i].second, b = breps[i].second;
            dfs_hashing(V, a, -1, atree, akids);
            dfs_hashing(V, b, -1, btree, bkids);
            dfs_matching(V, a, b, akids, bkids, ans);
        }
        return ans;
    }

    // Dfs postorder on all isomorphically distinct subtrees of the given tree
    // Only visits new hashes. Signature: (V, hash, edges[], tree[][])
    // Number of trees is O(2^(2V/3)) worst-case
    template <typename Fn>
    static bool visit_subtrees(int V, const Edges& edges, Fn&& visitor) {
        auto tree = make_tree(V, edges);
        auto [h, c] = hash_unrooted(V, 0, -1, tree);
        if (!visited_topologies.insert(h).second) {
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
    static inline unordered_map<pair<int, int>, int, Hasher> upper;
    using Edges = vector<array<int, 2>>;
    using Tree = vector<basic_string<int>>;

    // Build Tree from Edges
    static auto make_tree(int V, const Edges& edges) {
        vector<basic_string<int>> tree(V);
        for (auto [u, v] : edges) {
            tree[u].push_back(v);
            tree[v].push_back(u);
        }
        return tree;
    }

    // Build Edges from Tree
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

    static auto get(const vector<int>& hashes) {
        if (auto pos = cache.find(hashes); pos != end(cache)) {
            return make_pair(pos->second, false);
        } else {
            int S = cache.size();
            cache[hashes] = S;
            return make_pair(S, true);
        }
    }

    static auto get_upper(int p, int u) {
        if (auto pos = upper.find({p, u}); pos != end(upper)) {
            return make_pair(pos->second, false);
        } else {
            int S = upper.size();
            upper[{p, u}] = S;
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
            int h = get(hashes[u]).first;
            hashes[u].clear();
            hashes[parent[u]].push_back(h);
        }
        sort(begin(hashes[r]), end(hashes[r]));
        int h = get(hashes[r]).first;
        hashes[r].clear();
        return h;
    }

    static int hash_rooted(int r, int p, const Tree& tree) {
        return hash_rooted(tree.size(), r, p, tree);
    }

    static auto subt_hash_rooted(int r, int p, const Tree& tree) {
        static thread_local int S = 0;
        static thread_local vector<vector<int>> hashes;
        static thread_local vector<int> bfs, parent;
        int V = tree.size();
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
        vector<int> ans(V);
        for (int i = B - 1; i > 0; i--) {
            int u = bfs[i];
            sort(begin(hashes[u]), end(hashes[u]));
            ans[u] = get(hashes[u]).first;
            hashes[u].clear();
            hashes[parent[u]].push_back(ans[u]);
        }
        sort(begin(hashes[r]), end(hashes[r]));
        ans[r] = get(hashes[r]).first;
        hashes[r].clear();
        return ans;
    }

    static auto all_hash_rooted(int r, int p, const Tree& tree) {
        static thread_local int S = 0;
        static thread_local vector<vector<int>> hashes;
        static thread_local vector<int> bfs, parent;
        int V = tree.size();
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
        vector<int> ans(V), top(V);
        for (int i = B - 1; i > 0; i--) {
            int u = bfs[i];
            sort(begin(hashes[u]), end(hashes[u]));
            ans[u] = get(hashes[u]).first;
            hashes[u].clear();
            hashes[parent[u]].push_back(ans[u]);
        }
        sort(begin(hashes[r]), end(hashes[r]));
        ans[r] = get(hashes[r]).first;
        top[r] = ans[r];
        hashes[r].clear();
        for (int i = 1; i < B; i++) {
            int u = bfs[i];
            top[u] = get_upper(top[parent[u]], ans[u]).first;
        }
        return make_pair(move(ans), move(top));
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
            int a = r == u ? get({}).first : hash_rooted(V, r, u, tree);
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
            auto [h, ok] = get(hashes[u]);
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

// Generate all isomorphically distinct trees with at most N nodes and degree at most D
auto build_bounded_degree(int N, int D) {
    using Edges = vector<array<int, 2>>;
    vector<vector<Edges>> ans(N + 1);
    ans[1].push_back({});
    for (int V = 2; V <= N; V++) {
        unordered_set<int> seen;
        for (const Edges& G : ans[V - 1]) {
            vector<int> deg(V - 1);
            for (auto [u, v] : G) {
                deg[u]++, deg[v]++;
            }
            for (int u = 0; u < V - 1; u++) {
                if (deg[u] < D) {
                    Edges H = G;
                    H.push_back({u, V - 1});
                    int h = unrooted_topology::hash_tree(V, H);
                    if (seen.insert(h).second) {
                        ans[V].push_back(move(H));
                    }
                }
            }
        }
    }
    return ans;
}
