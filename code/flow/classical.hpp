#pragma once

#include "flow/dinitz_flow.hpp"
#include "flow/edmonds_karp.hpp"
#include "matching/bipartite_matching.hpp"
#include "matching/hopcroft_karp.hpp"
#include "algo/y_combinator.hpp"

// We have N projects. The i-th project yields projects[i] revenue if completed.
// We have M machines. The j-th machine costs machines[j] to build.
// The i-th project requires all machines in input[i] to be built so it can be completed.
// Find maximum revenue possible, the choice of projects and machines.
auto max_project_selection(const vector<int64_t>& projects,
                           const vector<int64_t>& machines,
                           const vector<vector<int>>& input) {
    using Solver = dinitz_flow<int64_t>;

    int N = projects.size(), M = machines.size();
    int s = N + M, t = s + 1;
    Solver mf(N + M + 2);

    for (int i = 0; i < N; i++) {
        mf.add(s, i, projects[i]);
    }
    for (int j = 0; j < M; j++) {
        mf.add(j + N, t, machines[j]);
    }
    for (int i = 0; i < N; i++) {
        for (int j : input[i]) {
            mf.add(i, j + N, Solver::flowinf);
        }
    }

    auto maxflow = mf.maxflow(s, t);
    int64_t revenue = 0;
    vector<int> took_machine(M);
    vector<int> choice_projects, choice_machines;

    for (int i = 0; i < N; i++) {
        bool satisfied = true;
        for (int j : input[i]) {
            satisfied &= mf.get_flow(j + N) == machines[j];
        }
        if (satisfied) {
            revenue += projects[i];
            for (int j : input[i]) {
                took_machine[j] = true;
            }
            choice_projects.push_back(i);
        }
    }
    for (int j = 0; j < M; j++) {
        if (took_machine[j]) {
            revenue -= machines[j];
            choice_machines.push_back(j);
        }
    }

    assert(revenue + maxflow == accumulate(begin(projects), end(projects), int64_t(0)));
    return make_tuple(revenue, move(choice_projects), move(choice_machines));
}

// We have N nodes, and we want to assign each to either side A or side B.
// Assigning node i to A yields A[i] profit, and assigning it to B yields B[i] profit.
// For several pairs (i,j) there is a penalty of c(i,j) if i,j are assigned differently.
// Find maximum profit and the assignment.
// This can be adjusted easily to support one-sided penalties.
auto max_segmentation(const vector<int64_t>& A, //
                      const vector<int64_t>& B,
                      const vector<tuple<int, int, int64_t>>& costs) {
    using Solver = dinitz_flow<int64_t>;

    assert(A.size() == B.size());
    int N = A.size();
    int s = N, t = N + 1;
    Solver mf(N + 2);

    for (int i = 0; i < N; i++) {
        mf.add(s, i, A[i]);
    }
    for (int i = 0; i < N; i++) {
        mf.add(i, t, B[i]);
    }
    for (auto [i, j, c] : costs) {
        assert(0 <= i && i < N && 0 <= j && j < N && i != j && c > 0);
        mf.add(i, j, c), mf.add(j, i, c);
    }

    int64_t maxflow = mf.maxflow(s, t);
    vector<int8_t> side(N);
    int64_t revenue = 0;

    for (int i = 0; i < N; i++) {
        side[i] = !mf.left_of_mincut(i);
        revenue += side[i] == 0 ? A[i] : B[i];
    }
    for (auto [i, j, c] : costs) {
        if (side[i] != side[j]) {
            revenue -= c;
        }
    }

    return make_pair(revenue, move(side));
}

// We have N teams in a competition. Team i already has w(i) wins.
// Given several tuples (u,v,c), there will be c games between teams u and v.
// Exactly one team wins each game, and all games will be played. A team is said to be
// eliminated if it cannot win the competition by finishing first, possibly tied.
// Determine if team k is eliminated. If so, return an elimination set; if not, return
// an assignment of wins to teams u, that lets team k win the competition.
auto single_elimination(const vector<int>& wins,
                        const vector<tuple<int, int, int>>& games, int k) {
    using Solver = dinitz_flow<int64_t>;

    int N = wins.size(), G = games.size();
    int64_t W = wins[k], F = 0;
    for (auto [u, v, c] : games) {
        W += u == k || v == k ? c : 0, F += c;
    }

    // Are we already beaten by the team currently leading the competition?
    int M = max_element(begin(wins), end(wins)) - begin(wins);
    if (W < wins[M]) {
        return make_tuple(true, vector<int>{M}, vector<int>{});
    }

    int s = N + G, t = s + 1;
    Solver mf(N + G + 2);

    for (int g = 0; g < G; g++) {
        auto [u, v, c] = games[g];
        assert(0 <= u && u < N && 0 <= v && v < N && u != v && c > 0);
        mf.add(s, N + g, c);
        mf.add(N + g, u, c);
        mf.add(N + g, v, c);
    }
    for (int u = 0; u < N; u++) {
        if (wins[u] < W) {
            mf.add(u, t, W - wins[u]);
        }
    }

    auto maxflow = mf.maxflow(s, t);

    if (maxflow == F) {
        vector<int> uwin(G);
        for (int g = 0; g < G; g++) {
            uwin[g] = mf.get_flow(3 * g + 1);
        }
        return make_tuple(false, vector<int>{}, move(uwin));
    } else {
        vector<int> elim;
        for (int u = 0; u < N; u++) {
            if (mf.left_of_mincut(u)) {
                elim.push_back(u);
            }
        }
        return make_tuple(true, move(elim), vector<int>{});
    }
}

// Determine a closure of G (subset of vertices with no outgoing edges) of maximum weight
auto max_weight_closure(const vector<array<int, 2>>& G, const vector<int64_t>& weight) {
    using Solver = dinitz_flow<int64_t>;

    int N = weight.size();
    int s = N, t = N + 1;
    Solver mf(N + 2);

    for (int u = 0; u < N; u++) {
        if (weight[u] > 0) {
            mf.add(s, u, weight[u]);
        } else if (weight[u] < 0) {
            mf.add(u, t, -weight[u]);
        }
    }
    for (auto [u, v] : G) {
        mf.add(u, v, Solver::flowinf);
    }

    return mf.maxflow(s, t);
}

// Determine maximum number of edge disjoint paths between s and t (sample)
auto max_edge_disjoint_paths(const vector<vector<int>>& out, int s, int t) {
    using Solver = edmonds_karp<int>;

    int N = out.size();
    Solver mf(N);

    for (int u = 0; u < N; u++) {
        for (int v : out[u]) {
            mf.add(u, v, 1);
        }
    }

    return mf.maxflow(s, t);
}

// Determine maximum number of vertex disjoint paths between s and t (sample)
auto max_vertex_disjoint_paths(int N, const vector<array<int, 2>>& G, int s, int t) {
    using Solver = edmonds_karp<int>;

    Solver mf(2 * N);

    for (int u = 0; u < N; u++) {
        mf.add(2 * u, 2 * u + 1, 1);
    }
    for (auto [u, v] : G) {
        mf.add(2 * u + 1, 2 * v, 1);
    }

    s = 2 * s + 1, t = 2 * t;
    return mf.maxflow(s, t);
}

// Decompose a directed acyclic graph into a minimum number of chains
auto min_dag_chain_decomposition(int N, const vector<array<int, 2>>& G) {
    hopcroft_karp bm(N, N);

    for (auto [u, v] : G) {
        bm.add(u, v);
    }

    int m = bm.max_matching();
    int C = N - m;
    vector<vector<int>> chains(C);

    for (int c = 0, u = 0; u < N; u++) {
        if (bm.mv[u] == -1) {
            int v = u;
            do {
                chains[c].push_back(v), v = bm.mu[v];
            } while (v != -1);
            c++;
        }
    }

    return make_tuple(C, move(chains), move(bm.mu), move(bm.mv));
}

// Construct bipartite maximum matching and compute minimum vertex cover from it
auto min_bipartite_vertex_cover(int N, int M, const vector<array<int, 2>>& G) {
    hopcroft_karp bm(N, M);

    for (auto [u, v] : G) {
        bm.add(u, v);
    }

    int m = bm.max_matching();
    vector<int> A, B;
    vector<int8_t> L(N), R(M);

    auto dfs = y_combinator([&](auto self, int u) -> void {
        L[u] = 1;
        for (int v : bm.adj[u]) {
            if (v != bm.mu[u] && !R[v]) {
                R[v] = 1, B.push_back(v);
                if (bm.mv[v] != -1) {
                    self(bm.mv[v]);
                }
            }
        }
    });

    for (int u = 0; u < N; u++) {
        if (L[u] == 0 && bm.mu[u] == -1) {
            dfs(u);
        }
    }
    for (int u = 0; u < N; u++) {
        if (L[u] == 0) {
            A.push_back(u);
        }
    }

    return make_tuple(m, move(A), move(B), move(bm.mu), move(bm.mv));
}
