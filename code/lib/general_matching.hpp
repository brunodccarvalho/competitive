#pragma once

#include "lib/graph_generator.hpp"

/**
 * Generate a random Erdos graph with maximum matching M.
 * This algorithm is not perfect; not all topologies can be generated, I think, and some
 * topologies will appear more frequently due to symmetry.
 *
 * To generate a general graph on V vertices with maximum matching M.
 * First add edges (2n,2n+1) for n=0...M-1. This is the canonical matching.
 * Then add more edges as follows:
 * - Case 1: intend to add E-M more edges (to get E total):
 *   - Do not add any edge (u,v) where u>=2M and v>=2M
 *   - We will add some of the following edges:
 *     - a: (2u,2v+1)   where 0<=u<M and 0<=v<M. (if u=v it already exists)
 *     - b: (2u,n)      where 0<=u<M and 2M<=n.
 *   * - a: M^2 edges | b: M(V-2M) edges
 *   - pick a partition of E-M into 4 parts with the upper cap above (*).
 *   - add the edges using pair_sample.
 * - Case 2. intend to add edges with probability p
 *   - Call a binomd to determine E and proceed with case 1.
 * Finally, relabel and shuffle to hide the topology.
 */
auto general_matching_max_edges(int V, int M) {
    assert(2 * M <= V);
    return 1L * M * (V - M);
}

auto general_matching_group_sizes(int V, int M, int E) {
    assert(2 * M <= V && M <= E && E <= general_matching_max_edges(V, M));

    long A = 1L * M * M - M; // M already matched
    long B = 1L * M * (V - 2 * M);

    auto s = partition_sample<long>(E - M, 2, {0, 0}, {A, B});
    int a = s[0], b = s[1];
    return array<int, 2>{a, b};
}

auto general_matching_hide_topology(int V, edges_t& g) {
    shuffle(begin(g), end(g), mt);
    random_relabel_graph_inplace(V, g);
    random_flip_graph_inplace(g);
}

auto random_general_matching(int V, int M, int E) {
    auto [a, b] = general_matching_group_sizes(V, M, E);
    edges_t g;
    g.reserve(E);

    for (int n = 0; n < M; n++)
        g.push_back({2 * n, 2 * n + 1});
    for (auto [u, v] : distinct_pair_sample(a, 0, M))
        g.push_back({2 * u, 2 * v + 1});
    for (auto [u, v] : pair_sample(b, 0, M, 2 * M, V))
        g.push_back({2 * u, v});

    return g;
}

auto random_general_matching(int V, int M, double p) {
    binomd distE(general_matching_max_edges(V, M), min(p, 1.0));
    int E = max(M, int(distE(mt)));
    return random_general_matching(V, M, E);
}
