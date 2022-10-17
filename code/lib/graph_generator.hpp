#pragma once

#include "hash.hpp"
#include "numeric/bits.hpp"
#include "numeric/math.hpp"
#include "random.hpp"
#include "graphs/regular.hpp"
#include "algo/y_combinator.hpp"
#include "lib/graph_operations.hpp"
#include "lib/max_geometric_distribution.hpp"

// ***** Auxiliary methods

using edges_t = vector<array<int, 2>>;

// Randomly permute node labels
void random_relabel_graph_inplace(int V, edges_t& g) {
    vector<int> label(V);
    iota(begin(label), end(label), 0);
    shuffle(begin(label), end(label), mt);
    for (auto& [u, v] : g)
        u = label[u], v = label[v];
}

void random_relabel_graph_inplace(int U, int V, edges_t& g) {
    vector<int> ulabel(U), vlabel(V);
    iota(begin(ulabel), end(ulabel), 0);
    iota(begin(vlabel), end(vlabel), 0);
    shuffle(begin(ulabel), end(ulabel), mt);
    shuffle(begin(vlabel), end(vlabel), mt);
    for (auto& [u, v] : g)
        u = ulabel[u], v = vlabel[v];
}

auto random_relabel_graph(int V, edges_t g) {
    return random_relabel_graph_inplace(V, g), g;
}

auto random_relabel_graph(int U, int V, edges_t g) {
    return random_relabel_graph_inplace(U, V, g), g;
}

// Randomly flip some edges with probability p
auto random_flip_graph_inplace(edges_t& g, double p = 0.5) {
    if (p <= 0.15) {
        for (int e : int_sample_p(p, 0, int(g.size())))
            swap(g[e][0], g[e][1]);
    } else {
        for (auto& [u, v] : g)
            if (cointoss(p))
                swap(u, v);
    }
}

// Add self loops to g each with uniform probability p
void add_uniform_self_loops(int V, edges_t& g, double p) {
    for (int u : int_sample_p(p, 0, V))
        g.push_back({u, u});
}

// merge the first S edges of undirected graph more that aren't in g into g
void add_edges_missing(edges_t& g, const edges_t& more, int S) {
    unordered_set<array<int, 2>> edgeset(begin(g), end(g));
    for (auto [u, v] : more) {
        if (S == 0)
            break;
        if (!edgeset.count({u, v}))
            g.push_back({u, v}), edgeset.insert({u, v}), S--;
    }
    assert(S == 0);
}

void add_edges_missing(edges_t& g, const edges_t& more) {
    unordered_set<array<int, 2>> edgeset(begin(g), end(g));
    for (auto [u, v] : more) {
        if (!edgeset.count({u, v}))
            g.push_back({u, v}), edgeset.insert({u, v});
    }
}

// fill the undirected subgraph [u1,u2) of g
void add_all_edges_forward(edges_t& g, int u1, int u2) {
    for (int u = u1; u < u2; u++)
        for (int v = u + 1; v < u2; v++)
            g.push_back({u, v});
}

// fill the directed subgraph [u1,u2) of g with edges oriented toward lower nodes
void add_all_edges_backward(edges_t& g, int u1, int u2) {
    for (int u = u1; u < u2; u++)
        for (int v = u + 1; v < u2; v++)
            g.push_back({v, u});
}

// fill the bipartite subgraph [u1,u2)x[v1,v2) of g
void add_all_edges_bipartite(edges_t& g, int u1, int u2, int v1, int v2) {
    for (int u = u1; u < u2; u++)
        for (int v = v1; v < v2; v++)
            g.push_back({u, v});
}

// add edges to directed g such that g becomes strongly connected
void add_tarjan_back_edges(edges_t& g, int V, int s = 0) {
    auto adj = make_adjacency_lists_directed(V, g);
    vector<int> start(V, -1), lowlink(V, -1), rev(V);
    int t = 0;
    y_combinator([&](auto self, int u) -> void {
        rev[t] = u, start[u] = lowlink[u] = t++;
        for (int v : adj[u]) {
            if (start[v] == -1) {
                self(v), lowlink[u] = min(lowlink[u], lowlink[v]);
            } else {
                lowlink[u] = min(lowlink[u], start[v]);
            }
        }
        if (start[u] > 0 && start[u] == lowlink[u]) {
            int x = intd(start[u], t - 1)(mt);
            int y = intd(0, start[u] - 1)(mt);
            int v = rev[x], w = rev[y];
            adj[v].push_back(w), g.push_back({v, w});
            lowlink[u] = y;
        }
    })(s);
}

// add missing edges to directed g, each with uniform probability p
void add_uniform_missing_directed(edges_t& g, int V, double p) {
    auto adj = make_adjacency_set_directed(g);
    for (auto [u, v] : distinct_pair_sample_p(p, 0, V))
        if (!adj.count({u, v}))
            g.push_back({u, v});
}

// *****

auto path_graph(int V) {
    assert(V > 0);
    edges_t g;
    for (int u = 0; u + 1 < V; u++)
        g.push_back({u, u + 1});
    return g;
}

auto cycle_graph(int V) {
    assert(V > 0);
    edges_t g;
    for (int u = 1; u < V; u++) {
        g.push_back({u - 1, u});
    }
    if (V >= 3)
        g.push_back({V - 1, 0});
    return g;
}

auto complete_graph(int V) {
    assert(V > 0 && V <= 5000);
    edges_t g;
    add_all_edges_forward(g, 0, V);
    return g;
}

auto complete2_graph(int V) { // add edges both ways
    assert(V > 0 && V <= 5000);
    edges_t g;
    add_all_edges_forward(g, 0, V);
    add_all_edges_backward(g, 0, V);
    return g;
}

auto complete_multipartite_graph(const vector<int>& R) {
    int V = accumulate(begin(R), end(R), 0);
    int start = 0, ranks = R.size();
    edges_t g;
    for (int r = 0; r < ranks; r++) {
        int mid = start + R[r];
        for (int u = start; u < mid; u++)
            for (int v = mid; v < V; v++)
                g.push_back({u, v});
        start = mid;
    }
    return g;
}

auto complete_bipartite_graph(int U, int V) {
    assert(U > 0 && V > 0 && 1LL * U * V <= 30'000'000);
    edges_t g;
    add_all_edges_bipartite(g, 0, U, 0, V);
    return g;
}

// *****

// n complete subgraphs each with k nodes
auto disjoint_complete_graph(int n, int k) {
    assert(n > 0 && k > 0 && 1LL * n * k * k <= 60'000'000);
    edges_t g;
    for (int i = 0; i < n; i++) {
        add_all_edges_forward(g, i * k, (i + 1) * k);
    }
    return g;
}

// n complete subgraphs each with k nodes
auto disjoint_complete2_graph(int n, int k) {
    assert(n > 0 && k > 0 && 1LL * n * k * k <= 30'000'000);
    edges_t g;
    for (int i = 0; i < n; i++) {
        add_all_edges_forward(g, i * k, (i + 1) * k);
        add_all_edges_backward(g, i * k, (i + 1) * k);
    }
    return g;
}

// links the last node of each component to the first node of the next component
auto one_connected_complete_graph(int n, int k) {
    assert(n > 0 && k > 0 && 1LL * n * k * k <= 60'000'000);
    auto g = disjoint_complete_graph(n, k);
    for (int i = 0; i + 1 < n; i++) {
        int u = (i + 1) * k - 1, v = (i + 1) * k;
        g.push_back({u, v});
    }
    return g;
}

// links the first node of each component to the first node of the next component
auto one_connected_complete2_graph(int n, int k) {
    assert(n > 0 && k > 0 && 1LL * n * k * k <= 30'000'000);
    auto g = disjoint_complete2_graph(n, k);
    for (int i = 0; i + 1 < n; i++) {
        int u = i * k, v = (i + 1) * k;
        g.push_back({u, v});
    }
    return g;
}

// links the ith node of each component to the ith node of the next component
auto k_connected_complete_graph(int n, int k, int links = INT_MAX) {
    assert(n > 0 && k > 0 && 1LL * n * k * k <= 60'000'000);
    links = clamp(links, 0, k);
    auto g = disjoint_complete_graph(n, k);
    for (int i = 0; i + 1 < n; i++) {
        int u = (i + 1) * k - links, v = (i + 1) * k;
        for (int j = 0; j < links; j++) {
            g.push_back({u + j, v + j});
        }
    }
    return g;
}

// links the ith node of each component to the ith node of the next component
auto k_connected_complete2_graph(int n, int k, int links = INT_MAX) {
    assert(n > 0 && k > 0 && 1LL * n * k * k <= 30'000'000);
    links = clamp(links, 0, k);
    auto g = disjoint_complete2_graph(n, k);
    for (int i = 0; i + 1 < n; i++) {
        int u = i * k, v = (i + 1) * k;
        for (int j = 0; j < links; j++) {
            g.push_back({u + j, v + j});
        }
    }
    return g;
}

// edge between u,v with u<v exists as u->v with probability forward_bias
auto random_tournament(int V, double forward_bias = 0.5) {
    assert(V > 0 && V <= 5000);
    edges_t g;
    g.reserve(V * (V - 1) / 2);
    for (int u = 0; u < V; u++)
        for (int v = u + 1; v < V; v++)
            cointoss(forward_bias) ? g.push_back({u, v}) : g.push_back({v, u});
    return g;
}

// *****

auto random_tree(int V) {
    assert(V > 0 && V <= 30'000'000);
    edges_t g;
    for (int u = 1; u < V; u++) {
        int p = intd(0, u - 1)(mt);
        g.push_back({p, u});
    }
    return g;
}

auto random_geometric_tree(int V, double alpha) {
    assert(V > 0 && V <= 30'000'000 && -1.0 < alpha && alpha < 1.0);
    edges_t g;
    for (int u = 1; u < V; u++) {
        g.push_back({rand_geom<int>(0, u - 1, -alpha), u});
    }
    return g;
}

auto random_binary_tree(int L) {
    vector<int> leader(L);
    iota(begin(leader), end(leader), 0);
    ordered_set<int> top(begin(leader), end(leader));
    edges_t g;
    int t = L, V = 2 * L - 1;
    while (L-- > 1) {
        int i = rand_unif<int>(0, L - 1);
        int a = *top.find_by_order(i);
        int b = *top.find_by_order(i + 1);
        top.erase(b);
        int u = leader[a];
        int v = leader[b];
        g.push_back({V - 1 - t, V - 1 - v});
        g.push_back({V - 1 - t, V - 1 - u});
        leader[a] = leader[b] = t++;
    }
    reverse(begin(g), end(g));
    return g;
}

auto random_geometric_binary_tree(int L, double alpha) {
    vector<int> leader(L);
    iota(begin(leader), end(leader), 0);
    ordered_set<int> top(begin(leader), end(leader));
    edges_t g;
    int t = L, V = 2 * L - 1;
    while (L-- > 1) {
        int i = rand_geom<int>(0, L - 1, alpha);
        int a = *top.find_by_order(i);
        int b = *top.find_by_order(i + 1);
        top.erase(b);
        int u = leader[a];
        int v = leader[b];
        g.push_back({V - 1 - t, V - 1 - v});
        g.push_back({V - 1 - t, V - 1 - u});
        leader[a] = leader[b] = t++;
    }
    reverse(begin(g), end(g));
    return g;
}

auto random_periodic_tree(int V, int period, double alpha, double beta = 0.75) {
    edges_t g;
    double off = rand_unif<double>(0.0, period);
    int sub = max<int>(2, ceil(period * beta));
    for (int u = 1; u < V; u++) {
        double p = cos((u + off) * 2.0 * M_PI / period) * alpha;
        g.push_back({rand_geom<int>(max(0, u - sub), u - 1, p), u});
    }
    return g;
}

auto random_forest(const vector<int>& tree_sizes) {
    edges_t g;
    g.reserve(accumulate(begin(tree_sizes), end(tree_sizes), 0));
    int T = tree_sizes.size();
    for (int i = 0, s = 0; i < T; s += tree_sizes[i++]) {
        for (auto [u, v] : random_tree(tree_sizes[i])) {
            g.push_back({u + s, v + s});
        }
    }
    return g;
}

auto random_forest(int V, int trees, int min_tree_size = 1) {
    assert(V > 0 && V <= 30'000'000);
    auto tree_sizes = partition_sample(V, trees, min_tree_size);
    return random_forest(tree_sizes);
}

auto random_geometric_forest(const vector<int>& tree_sizes, double alpha) {
    assert(-1.0 < alpha && alpha < 1.0);
    edges_t g;
    g.reserve(accumulate(begin(tree_sizes), end(tree_sizes), 0));
    int T = tree_sizes.size();
    for (int i = 0, s = 0; i < T; s += tree_sizes[i++]) {
        for (auto [u, v] : random_geometric_tree(T, alpha)) {
            g.push_back({u + s, v + s});
        }
    }
    return g;
}

auto random_geometric_forest(int V, int trees, double alpha, int min_tree_size = 1) {
    assert(V > 0 && V <= 30'000'000 && -1.0 < alpha && alpha < 1.0);
    auto tree_sizes = partition_sample(V, trees, min_tree_size);
    return random_geometric_forest(tree_sizes, alpha);
}

// *****

auto random_regular(int V, int k) {
    return regular_sample(V, k); // Just rename
}

auto random_regular_connected(int V, int k) {
    edges_t g;
    do {
        g = random_regular(V, k);
    } while (!is_connected_undirected(g, V));
    return g;
}

auto random_regular_directed(int V, int k) {
    return regular_directed_sample(V, k); // Just rename
}

auto random_regular_directed_connected(int V, int k) {
    edges_t g;
    do {
        g = random_regular_directed(V, k);
    } while (!is_connected_directed(g, V));
    return g;
}

auto random_regular_bipartite(int U, int V, int k) {
    return regular_bipartite_sample(U, V, k); // Just rename
}

// *****

// edge u->v, u<v, exists with probability p
auto random_uniform_undirected(int V, double p) {
    assert(0.0 < p && p <= 1.0);
    return choose_sample_p(p, 0, V); //
}

// edge u->v exists with probability p
auto random_uniform_directed(int V, double p) {
    assert(0.0 < p && p <= 1.0);
    return distinct_pair_sample_p(p, 0, V); //
}

// edge u->v exists with probability p
auto random_uniform_bipartite(int U, int V, double p) {
    assert(0.0 < p && p <= 1.0);
    return pair_sample_p(p, 0, U, 0, V); //
}

// *****

auto get_geometric_parameters(int V, double p, double alpha) {
    assert(V > 0 && 0.0 < alpha && alpha < 1.0 && 0.0 < p && p <= 1.0);
    constexpr double epsilon = 10 * numeric_limits<double>::epsilon();
    double E = p * V * (V - 1) / 2;
    for (int n = 0; n < V - 1; n++) {
        double one = 1.0 * n * (2 * V - n - 1) / 2;
        double geo = (pow(1 - alpha, V - n) + alpha * (V - n) - 1) / (alpha * alpha);
        assert(one < E + epsilon && geo > 0);
        if (one + geo >= E) {
            // reduce geo to p*geo so that one + p*geo = E <==> p = (E - one) / geo
            if (geo <= epsilon || one >= E) {
                return make_tuple(n, 1.0);
            } else {
                return make_tuple(n, clamp((E - one) / geo, 0.0, 1.0));
            }
        }
    }
    return make_tuple(V - 1, 1.0);
}

/**
 * Geometric undirected with edges u->v, u<v
 * If alpha>0, deep graph, edge u->v exists with probability ~ (1-alpha)^(v-u)
 * If alpha<0, wide graph, edge u->v exists with probability ~ (1+alpha)^u
 * If alpha=0, uniform
 * The implementation will amortize these probabilities so that each possible edge
 * exists on average with probability p
 */
auto random_geometric_undirected(int V, double p, double alpha) {
    assert(V > 0 && -1.0 < alpha && alpha < 1.0 && 0.0 < p && p <= 1.0);
    edges_t g;
    if (alpha > 0.0) { // deep
        // sure edges for d=[1..n]
        auto [n, f] = get_geometric_parameters(V, p, alpha);
        for (int d = 1; d <= n; d++) {
            for (int u = 0; u < V - d; u++)
                g.push_back({u, u + d});
        }
        for (int d = n + 1; d < V; d++) {
            double q = d > n + 1 ? f * pow(1 - alpha, d - n - 1) : f;
            for (int u : int_sample_p(q, 0, V - d))
                g.push_back({u, u + d});
        }
    } else if (alpha < 0.0) { // wide
        // sure edges for u=[0..n)
        auto [n, f] = get_geometric_parameters(V, p, -alpha);
        for (int u = 0; u < n; u++) {
            for (int v = u + 1; v < V; v++)
                g.push_back({u, v});
        }
        for (int u = n; u < V; u++) {
            double q = u > n ? f * pow(1 + alpha, u - n) : f;
            for (int v : int_sample_p(q, u + 1, V))
                g.push_back({u, v});
        }
    } else {
        g = random_uniform_undirected(V, p);
    }
    return g;
}

/**
 * Geometric undirected with edges u->v, u<v or u>v
 * If alpha>0, deep graph, edge u<->v exists with probability p*(1-alpha)^(|v-u|+beta)
 * If alpha<0, wide graph, edge u<->v exists with probability p*(1+alpha)^(u+1+beta)
 * If alpha=0, uniform
 * 1+-alpha is the probability for the 'base' edges (u<->u+1 when deep, 0<->u when wide)
 * and the probability decays by alpha.
 */
auto random_geometric_directed(int V, double p, double alpha) {
    assert(V > 0 && -1.0 < alpha && alpha < 1.0 && 0.0 < p && p <= 1.0);
    edges_t g;
    if (alpha > 0.0) { // deep
        // sure edges for d=[1..n]
        auto [n, f] = get_geometric_parameters(V, p, alpha);
        for (int d = 1; d <= n; d++) {
            for (int u = 0; u < V - d; u++)
                g.push_back({u, u + d});
            for (int u = 0; u < V - d; u++)
                g.push_back({u + d, u});
        }
        for (int d = n + 1; d < V; d++) {
            double q = d > n + 1 ? f * pow(1 - alpha, d - n - 1) : f;
            for (int u : int_sample_p(q, 0, V - d))
                g.push_back({u, u + d});
            for (int u : int_sample_p(q, 0, V - d))
                g.push_back({u + d, u});
        }
    } else if (alpha < 0.0) { // wide
        // sure edges for u=[0..n)
        auto [n, f] = get_geometric_parameters(V, p, -alpha);
        for (int u = 0; u < n; u++) {
            for (int v = u + 1; v < V; v++)
                g.push_back({u, v});
            for (int v = u + 1; v < V; v++)
                g.push_back({v, u});
        }
        for (int u = n; u < V; u++) {
            double q = u > n ? f * pow(1 + alpha, u - n) : f;
            for (int v : int_sample_p(q, u + 1, V))
                g.push_back({u, v});
            for (int v : int_sample_p(q, u + 1, V))
                g.push_back({v, u});
        }
    } else {
        g = random_uniform_directed(V, p);
    }
    return g;
}

// *****

auto random_exact_undirected(int V, int E) {
    return choose_sample(E, 0, V); //
}

auto random_exact_directed(int V, int E) {
    return distinct_pair_sample(E, 0, V); //
}

auto random_exact_bipartite(int U, int V, int E) {
    return pair_sample(E, 0, U, 0, V); //
}

// *****

auto random_uniform_undirected_total(int V, double p) {
    edges_t g = random_uniform_undirected(V, p);
    vector<int8_t> degree(V);
    for (auto [u, v] : g) {
        degree[u] = degree[v] = 1;
    }
    for (int u = 0; u < V; u++) {
        if (degree[u] == 0) {
            int v = diff_unif<int>(u, 0, V - 1);
            degree[v] = 1;
            u < v ? g.push_back({u, v}) : g.push_back({v, u});
        }
    }
    return g;
}

auto random_uniform_directed_total_out(int V, double p) {
    edges_t g = random_uniform_directed(V, p);
    vector<int8_t> degree(V);
    for (auto [u, v] : g) {
        degree[u] = 1;
    }
    for (int u = 0; u < V; u++) {
        if (degree[u] == 0) {
            int v = diff_unif<int>(u, 0, V - 1);
            g.push_back({u, v});
        }
    }
    return g;
}

auto random_uniform_directed_total_in(int V, double p) {
    edges_t g = random_uniform_directed(V, p);
    vector<int8_t> degree(V);
    for (auto [u, v] : g) {
        degree[v] = 1;
    }
    for (int v = 0; v < V; v++) {
        if (degree[v] == 0) {
            int u = diff_unif<int>(v, 0, V - 1);
            g.push_back({u, v});
        }
    }
    return g;
}

auto random_uniform_bipartite_total(int U, int V, double p) {
    edges_t g = random_uniform_bipartite(U, V, p);
    vector<int8_t> out(U), in(V);
    for (auto [u, v] : g)
        assert(u < V && v < V), out[u] = in[v] = 1;
    for (int u = 0; u < U; u++) {
        if (out[u] == 0) {
            int v = intd(0, V - 1)(mt);
            g.push_back({u, v});
            in[v] = 1;
        }
    }
    for (int v = 0; v < V; v++) {
        if (in[v] == 0) {
            int u = intd(0, U - 1)(mt);
            g.push_back({u, v});
        }
    }
    return g;
}

auto random_uniform_unrooted_dag_total(int V, double p) {
    return random_uniform_undirected_total(V, p); // :-)
}

auto random_uniform_undirected_connected(int V, double p) {
    edges_t g;
    vector<int> parent(V);
    for (int u = 1; u < V; u++) {
        parent[u] = intd(0, u - 1)(mt);
        g.push_back({parent[u], u});
    }
    p = clamp(p - 1.0 * g.size() / V / V, 0.0, 1.0);
    for (int v = 1; v < V && p > 0.0; v++) {
        for (int u : int_sample_p(p, 0, v))
            if (u != parent[v])
                g.push_back({u, v});
    }
    return g;
}

auto random_uniform_directed_connected(int V, double p) {
    edges_t g;
    for (int u = 1; u < V; u++) {
        int a = intd(0, u - 1)(mt);
        g.push_back({a, u});
    }
    add_tarjan_back_edges(g, V);
    p = clamp(p - 1.0 * g.size() / V / V, 0.0, 1.0);
    add_uniform_missing_directed(g, V, p);
    return g;
}

// *****

auto random_exact_undirected_connected(int V, int E) {
    assert(V - 1 <= E && E <= 1L * V * (V - 1) / 2);
    edges_t g;
    vector<int> parent(V);
    for (int u = 1; u < V; u++) {
        parent[u] = intd(0, u - 1)(mt);
        g.push_back({parent[u], u});
    }
    if (E == int(g.size())) {
        return g;
    }
    auto more = choose_sample(E, 0, V);
    shuffle(begin(more), end(more), mt);
    add_edges_missing(g, more, E - (V - 1));
    return g;
}

auto random_exact_rooted_dag_connected(int V, int E) {
    assert(V - 1 <= E && E <= 1L * V * (V - 1) / 2);
    edges_t g;
    vector<int> parent(V);
    for (int u = 1; u < V; u++) {
        parent[u] = intd(0, u - 1)(mt);
        g.push_back({parent[u], u});
    }
    if (E == int(g.size())) {
        return g;
    }
    int k = min(1L * V * (V - 1) / 2, 0L + E + V);
    auto more = choose_sample(k, 0, V);
    shuffle(begin(more), end(more), mt);
    add_edges_missing(g, more, E - (V - 1));
    return g;
}

auto random_exact_directed_connected(int V, int E) {
    assert(2 * V - 2 <= E && E <= 1L * V * (V - 1));
    edges_t g;
    vector<int> parent(V);
    for (int u = 1; u < V; u++) {
        parent[u] = intd(0, u - 1)(mt);
        g.push_back({parent[u], u});
    }
    add_tarjan_back_edges(g, V);
    int n = g.size();
    assert(n <= E);
    if (E == n) {
        return g;
    }
    auto f = distinct_pair_sample(E, 0, V);
    shuffle(begin(f), end(f), mt);
    add_edges_missing(g, f, E - n);
    return g;
}

// edge u->v, u<v, exists with probability p*alpha^(u+1+beta), bootstrap geometric tree
auto random_geometric_undirected_connected(int V, double p, double alpha) {
    assert(V > 0 && -1.0 < alpha && alpha < 1.0 && 0.0 < p && p <= 1.0);
    auto g = random_geometric_tree(V, alpha);
    auto h = random_geometric_undirected(V, p, alpha);
    add_edges_missing(g, h);
    return g;
}

// *****

// from s we can reach every node, and from every node we can reach t
auto random_uniform_flow_dag_connected(int V, double p) {
    auto f = random_uniform_undirected_connected(V, p / 2);
    auto r = random_uniform_undirected_connected(V, p / 2);
    unordered_set<array<int, 2>> edgeset(begin(f), end(f));
    for (auto [a, b] : r) {
        edgeset.insert({V - 1 - b, V - 1 - a});
    }
    edges_t g(begin(edgeset), end(edgeset));
    return make_tuple(g, 0, V - 1);
}

auto random_geometric_flow_dag_connected(int V, double p, double alpha) {
    auto f = random_geometric_undirected_connected(V, p / 2, alpha);
    auto r = random_geometric_undirected_connected(V, p / 2, alpha);
    unordered_set<array<int, 2>> edgeset(begin(f), end(f));
    for (auto [a, b] : r) {
        edgeset.insert({V - 1 - b, V - 1 - a});
    }
    edges_t g(begin(edgeset), end(edgeset));
    return make_tuple(g, 0, V - 1);
}

// from s we can reach every node, and from every node we can reach t, with backedges also
auto random_uniform_flow_connected(int V, double forward_p, double backward_p) {
    auto g = random_uniform_flow_dag_connected(V, forward_p);
    auto h = random_uniform_undirected(V, backward_p);
    for (auto [u, v] : h) {
        get<0>(g).push_back({v, u});
    }
    return g;
}

auto random_geometric_flow_connected(int V, double forward_p, double backward_p,
                                     double alpha) {
    auto g = random_geometric_flow_dag_connected(V, forward_p, alpha);
    auto h = random_geometric_undirected(V, backward_p, alpha);
    for (auto [u, v] : h) {
        get<0>(g).push_back({v, u});
    }
    return g;
}
