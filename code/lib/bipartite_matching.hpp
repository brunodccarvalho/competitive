#pragma once

#include "lib/graph_generator.hpp"

/**
 * Generate a random Erdos bipartite graph with maximum matching M.
 * This algorithm is not perfect; not all topologies can be generated, I think, and some
 * topologies will appear more frequently due to symmetry.
 *
 * To generate a bipartite graph on (U,V) vertices with maximum matching M.
 * First add edges (n,n) for n=0...M-1. This is the canonical matching.
 * Then add more edges as follows:
 * - Case 1: intend to add E-M more edges (to get E total):
 *   - Do not add any edge (u,v) where u>=M and v>=M
 *   - Pick some m in 0..M. We will add some of the following edges:
 *     - a: (u,v) where 0<=u<m and 0<=v<m. (if u=v it already exists)
 *     - b: (u,v) where m<=u<M and m<=v<M. (if u=v it already exists)
 *     - c: (u,v) where 0<=u<m and M<=v.
 *     - d: (u,v) where M<=u   and m<=v<M
 *   * - a: m^2 edges | b: (M-m)^2 edges | c: m(V-M) edges | d: (M-m)(U-M) edges
 *   - Pick a partition of E-M into 4 parts with the upper cap above (*).
 *   - Add the edges using pair_sample.
 * - Case 2. intend to add edges with probability p
 *   - Call a binomd to determine E and proceed with case 1.
 * Finally, relabel and shuffle to hide the topology.
 */
auto bipartite_matching_max_edges(int U, int V, int M) {
    assert(M <= U && M <= V);
    return 1L * M * max(U, V);
}

auto bipartite_matching_group_sizes(int U, int V, int M, int E) {
    assert(M <= U && M <= V && M <= E && E <= bipartite_matching_max_edges(U, V, M));
    vector<int> okm = {0};
    for (int m = 1; m <= M; m++) {
        int64_t A = 1LL * m * m - m;                   // m already matched
        int64_t B = 1LL * (M - m) * (M - m) - (M - m); // M - m already matched
        int64_t C = 1LL * m * (V - M);
        int64_t D = 1LL * (M - m) * (U - M);
        if (A >= 0 && B >= 0 && C >= 0 && D >= 0 && A + B + C + D >= E - M) {
            okm.push_back(m);
        }
    }

    int S = okm.size();
    int m = okm[intd(0, S - 1)(mt)];
    int64_t A = 1LL * m * m - m;                   // m already matched
    int64_t B = 1LL * (M - m) * (M - m) - (M - m); // M - m already matched
    int64_t C = 1LL * m * (V - M);
    int64_t D = 1LL * (M - m) * (U - M);

    auto s = partition_sample<int64_t>(E - M, 4, {0, 0, 0, 0}, {A, B, C, D});
    int a = s[0], b = s[1], c = s[2], d = s[3];
    return make_tuple(m, a, b, c, d);
}

auto bipartite_matching_hide_topology(int U, int V, edges_t& g) {
    shuffle(begin(g), end(g), mt);
    random_relabel_graph_inplace(U, V, g);
}

auto random_bipartite_matching(int U, int V, int M, int E) {
    auto [m, a, b, c, d] = bipartite_matching_group_sizes(U, V, M, E);
    edges_t g;
    g.reserve(E);

    for (int n = 0; n < M; n++)
        g.push_back({n, n});
    for (auto [u, v] : distinct_pair_sample(a, 0, m))
        g.push_back({u, v});
    for (auto [u, v] : distinct_pair_sample(b, m, M))
        g.push_back({u, v});
    for (auto [u, v] : pair_sample(c, 0, m, M, V))
        g.push_back({u, v});
    for (auto [u, v] : pair_sample(d, M, U, m, M))
        g.push_back({u, v});

    return g;
}

auto random_bipartite_matching(int U, int V, int M, double p) {
    binomd distE(bipartite_matching_max_edges(U, V, M), clamp(p, 0.0, 1.0));
    int E = max(M, int(distE(mt)));
    return random_bipartite_matching(U, V, M, E);
}
