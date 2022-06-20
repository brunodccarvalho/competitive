#pragma once

#include "flow/dinitz_flow.hpp"
#include "flow/edmonds_karp.hpp"
#include "flow/network_simplex.hpp"
#include "matching/hopcroft_karp.hpp"
#include "algo/y_combinator.hpp"

// --- Bipartite matching

// Determine maximum number of edge disjoint paths between s and t (sample)
auto max_edge_disjoint_paths(int N, const vector<array<int, 2>>& G, int s, int t) {
    using Solver = edmonds_karp<int>;

    Solver mf(N);

    for (auto [u, v] : G) {
        mf.add(u, v, 1);
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

// --- Maxflow

template <typename Solver>
auto decompose_dag_flow_matching(Solver& mf, int s, int t) {
    int V = mf.V;
    vector<int> reach = {s}, indeg(V);
    for (int i = 0, R = 1; i < R; i++) {
        for (int e : mf.res[reach[i]]) {
            if (int v = mf.edge[e].node[1]; !(e & 1) && mf.edge[e].flow && v != s) {
                if (v != t && indeg[v]++ == 0) {
                    reach.push_back(v), R++;
                }
            }
        }
    }
    vector<vector<array<int, 2>>> edgepaths(V);
    vector<int> bfs = {s};
    for (int i = 0, B = 1; i < B; i++) {
        int u = bfs[i];
        for (int e : mf.res[u]) {
            if (int v = mf.edge[e].node[1]; !(e & 1) && mf.edge[e].flow && v != s) {
                int k = mf.edge[e].flow;
                while (k--) {
                    if (u == s) {
                        edgepaths[v].push_back({e / 2, -1});
                    } else {
                        int f = edgepaths[u].back()[0];
                        edgepaths[u].pop_back();
                        edgepaths[v].push_back({f, e / 2});
                    }
                }
                if (v != t && --indeg[v] == 0) {
                    bfs.push_back(v), B++;
                }
            }
        }
    }
    return edgepaths[t];
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
        if (i != j && c > 0) {
            mf.add(i, j, c), mf.add(j, i, c);
        }
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
// Given (u,v,c), there will be c games between teams u and v.
// Exactly one team wins each game, and all games will be played.
// A team is eliminated if it can't win the competition by finishing first.
// Determine if team k is eliminated. If so, return an elimination set;
//     if not, return an assignment of wins to teams u, that lets team k win.
// Allow ties for first with strict=false
auto single_elimination(const vector<int>& wins,
                        const vector<tuple<int, int, int>>& games, int k, bool strict) {
    using Solver = dinitz_flow<int64_t>;

    int N = wins.size(), G = games.size();
    int64_t W = wins[k], F = 0;
    for (auto [u, v, c] : games) {
        W += u == k || v == k ? c : 0, F += c;
    }
    int64_t M = strict ? W - 1 : W;

    // Are we already beaten by the team currently leading the competition?
    for (int i = 0; i < N; i++) {
        if (i != k && wins[i] > M) {
            return make_tuple(true, vector<int>{i}, vector<int>{});
        }
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
        if (u == k && wins[u] < W) {
            mf.add(u, t, W - wins[u]);
        } else if (u != k && wins[u] < M) {
            mf.add(u, t, M - wins[u]);
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

    int64_t positive = 0;

    for (int u = 0; u < N; u++) {
        if (weight[u] > 0) {
            mf.add(s, u, weight[u]);
            positive += weight[u];
        } else if (weight[u] < 0) {
            mf.add(u, t, -weight[u]);
        }
    }
    for (auto [u, v] : G) {
        mf.add(u, v, Solver::flowinf);
    }

    auto maxflow = mf.maxflow(s, t);
    return positive - maxflow;
}

// Determine maximum weight longest antichain
auto max_weight_antichain(const vector<array<int, 2>>& G, const vector<int64_t>& weight) {
    using Solver = dinitz_flow<int64_t>;

    int N = weight.size();
    int64_t B = *max_element(begin(weight), end(weight));
    int64_t big = B * N + 1;

    int s = 2 * N, t = 2 * N + 1;
    Solver mf(2 * N + 2);
    vector<int> in(N), out(N);

    for (int u = 0; u < N; u++) {
        assert(weight[u] >= 0);
        mf.add(2 * u, 2 * u + 1, big - weight[u]);
    }
    for (auto [u, v] : G) {
        mf.add(2 * u + 1, 2 * v, Solver::flowinf);
        out[u]++, in[v]++;
    }
    for (int u = 0; u < N; u++) {
        if (in[u] == 0) {
            mf.add(s, 2 * u, Solver::flowinf);
        }
        if (out[u] == 0) {
            mf.add(2 * u + 1, t, Solver::flowinf);
        }
    }

    auto maxflow = mf.maxflow(s, t);
    vector<int> antichain;
    for (int u = 0; u < N; u++) {
        if (mf.left_of_mincut(2 * u) && !mf.left_of_mincut(2 * u + 1)) {
            antichain.push_back(u);
        }
    }
    int L = antichain.size();
    return make_tuple(L, L * big - maxflow, move(antichain));
}

// We have N people, C clubs and P parties. The i-th person belongs to party[i].
// Each person belongs to at least 1 club, there are (club,person) pairs.
// Each club must nominate a member to represent it, and at most portion[p] for party p.
// What is the largest balanced set of representatives possible?
auto balanced_representatives(int N, int C, int P, const vector<int>& party,
                              const vector<int>& portion,
                              const vector<array<int, 2>>& members) {
    int s = N + C + P, t = s + 1;
    dinitz_flow<int> mf(N + C + P + 2);

    for (auto [c, u] : members) {
        mf.add(c, u + C, 1);
    }
    for (int c = 0; c < C; c++) {
        mf.add(s, c, 1);
    }
    for (int u = 0; u < N; u++) {
        mf.add(u + C, party[u] + C + N, 1);
    }
    for (int p = 0; p < P; p++) {
        mf.add(p + C + N, t, portion[p]);
    }

    auto maxflow = mf.maxflow(s, t);

    vector<int> rep(C, -1);
    for (int e = 0, E = members.size(); e < E; e++) {
        if (mf.get_flow(e)) {
            auto [c, u] = members[e];
            rep[c] = u;
        }
    }

    return make_pair(maxflow, move(rep));
}

// We have a matrix A with real entries. Construct a matrix R such that R[i][j] is either
// the lower or upper rounding of A[i][j], and such that the row sums and column sums of
// R are also the lower or upper rounding of the row sums and column sums of A.
auto matrix_rounding(const vector<vector<double>>& A) {
    int N = A.size(), M = A[0].size(), s = N * M, t = s + 1;
    circulation<int64_t> circ(N + M + 2);

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int64_t low = floor(A[i][j]);
            circ.add(i, j + N, low, low + 1);
        }
    }
    for (int i = 0; i < N; i++) {
        double sum = 0;
        for (int j = 0; j < M; j++) {
            sum += A[i][j];
        }
        int64_t low = floor(sum);
        circ.add(s, i, low, low + 1);
    }
    for (int j = 0; j < M; j++) {
        double sum = 0;
        for (int i = 0; i < N; i++) {
            sum += A[i][j];
        }
        int64_t low = floor(sum);
        circ.add(j + N, t, low, low + 1);
    }

    constexpr int64_t inf = INT64_MAX / 4;
    circ.add(t, s, -inf, inf);

    auto [ok, flow] = circ.feasible_circulation();

    vector<vector<int64_t>> R(N, vector<int64_t>(M));
    for (int i = 0, e = 0; i < N; i++) {
        for (int j = 0; j < M; j++, e++) {
            R[i][j] = circ.get_flow(e);
        }
    }
    return R;
}

// --- Mincost flow

// There is a deck collection holding cards with labels 0 through N-1.
// We currently have start[i] cards with label i, and we want to get goal[i] such cards.
// Given (u,v,c,s), we can swap a card u for a card v at cost c and at most s times.
// What is the minimum cost to reach our goal, if it is possible?
auto min_cards_swap(int N, const vector<int>& start, const vector<int>& goal,
                    const vector<tuple<int, int, int, int>>& swaps) {
    network_simplex<int64_t, int64_t> ns(N);

    for (int u = 0; u < N; u++) {
        ns.set_supply(u, start[u] - goal[u]);
    }
    for (auto [u, v, c, s] : swaps) {
        if (u != v) {
            ns.add(u, v, 0, s, c);
        }
    }

    if (ns.mincost_circulation()) {
        return make_pair(true, ns.get_circulation_cost());
    } else {
        return make_pair(false, int64_t(0));
    }
}

// Minimum chinese postman tour, i.e. walk visiting every edge, weighted edges
auto min_chinese_postman(int N, const vector<array<int, 2>>& G, const vector<int>& cost) {
    network_simplex<int, int64_t> ns(N);

    int E = G.size();
    for (int e = 0; e < E; e++) {
        auto [u, v] = G[e];
        assert(cost[e] > 0);
        ns.add(u, v, 1, 2 * N, cost[e]);
    }

    if (ns.mincost_circulation()) {
        return make_pair(true, ns.get_circulation_cost());
    } else {
        return make_pair(false, int64_t(0));
    }
}

// Shortest chinese postman tour, i.e. shortest walk visiting every edge at least once
auto shortest_chinese_postman(int N, const vector<array<int, 2>>& G) {
    network_simplex<int, int64_t> ns(N);

    int E = G.size();
    for (int e = 0; e < E; e++) {
        auto [u, v] = G[e];
        ns.add(u, v, 1, 2 * N, 1);
    }

    if (ns.mincost_circulation()) {
        return make_pair(true, ns.get_circulation_cost());
    } else {
        return make_pair(false, int64_t(0));
    }
}

// A train moves from station 0 to station N, and has a capacity of P users.
// For (i,j,c,p), the company sells tickets from station i to station j at a cost of c.
//     per ticket, and there are p people who are interested in this ticket.
// What is the maximum profit possible?
auto max_train_allocation(int N, int P,
                          const vector<tuple<int, int, int, int>>& tickets) {
    int T = tickets.size();
    network_simplex<int, int64_t> ns(N + 1);

    for (auto [i, j, c, p] : tickets) {
        ns.add(j, i, 0, p, -c);
    }
    for (int i = 0; i < N; i++) {
        ns.add(i, i + 1, 0, P, 0);
    }

    auto flow = ns.mincost_flow();
    auto cost = ns.mincost_circulation();

    vector<int> sold(T);
    for (int e = 0; e < T; e++) {
        sold[e] = ns.get_flow(e);
    }
    return make_tuple(cost, flow, move(sold));
}

// We have a period of N days. On the i-th day we wish to meet demand of demand[i] items.
// Given (i,j,c,s), we can hire up to s workers, each to produce one item per day from
//     day i to day j inclusive, each worker at a hire cost of c.
// Can we meet all demand? And what is the minimum cost?
auto employment_scheduling(int N, int M, const vector<int>& demand,
                           const vector<tuple<int, int, int, int>>& workers) {
    int64_t D = *max_element(begin(demand), end(demand));
    int K = N + 1, W = workers.size();
    network_simplex<int64_t, int64_t> ns(2 * K + M);

    for (auto [i, j, c, s] : workers) {
        ns.add(j + 1, i, 0, s, c);
    }
    for (int i = 0; i < N; i++) {
        ns.add(i, i + K, demand[i], D, 0);
        ns.add(i + K, i + 1, 0, D, 0);
    } // pretty much the same as train allocation

    if (ns.mincost_circulation()) {
        vector<int> hires(W);
        for (int e = 0; e < W; e++) {
            hires[e] = ns.get_flow(e);
        }
        return make_tuple(true, ns.get_circulation_cost(), move(hires));
    } else {
        return make_tuple(false, int64_t(0), vector<int>(W));
    }
}

// Solve a min linear program with consecutive 1s in columns by converting it to a min
// cost flow problem. Variables are integers with individual lower and upper bounds.
// Constraints are individually either >= or <=.
auto lp_consecutive_ones_columns(const vector<vector<int>>& A, const vector<int>& B,
                                 const vector<int>& C, const vector<int>& lower,
                                 const vector<int>& upper, const vector<bool>& geq) {
    int N = C.size(), M = B.size();
    assert(B.size() == A.size() && C.size() == A[0].size());

    vector<int> first(N + M, -1), last(N + M, -1);
    for (int j = 0; j < N; j++) {
        for (int i = 0; i < M; i++) {
            if (last[j] != -1 && A[i][j] != 0)
                throw runtime_error("Invalid system");
            if (A[i][j] == 0 && last[j] == -1 && first[j] != -1)
                last[j] = i + 1;
            if (A[i][j] == 1 && first[j] == -1)
                first[j] = i;
        }
    }
    for (int i = 0; i < M; i++) {
        if (geq[i]) {
            first[i + N] = i + 1, last[i + N] = i;
        } else {
            first[i + N] = i, last[i + N] = i + 1;
        }
    }

    constexpr int inf = 1e9;
    network_simplex<int64_t, int64_t> ns(M + N + 1);

    for (int i = 0; i <= M; i++) {
        ns.add_supply(i, i < M ? B[i] : 0);
        ns.add_demand(i, i > 0 ? B[i - 1] : 0);
    }
    for (int i = 0; i < N; i++) {
        ns.add(first[i], last[i], lower[i], upper[i], C[i]);
    }
    for (int i = N; i < N + M; i++) {
        ns.add(first[i], last[i], 0, inf, 0);
    }

    if (ns.mincost_circulation()) {
        vector<int> x(N);
        for (int i = 0; i < N; i++) {
            x[i] = ns.get_flow(i);
        }
        return make_tuple(true, ns.get_circulation_cost(), move(x));
    } else {
        return make_tuple(false, int64_t(0), vector<int>{});
    }
}

// --- Greedies

// We have a period of N days. On the i-th day we wish to meet demand of demand[i] items.
// On the i-th day we can produce up to supply[i] items, each for cost[i] dollars.
// The items stick around for later days, i.e. they can be produced and consumed later.
// Can we meet all demand? And what is the minimum cost? O(N log N)
auto unbounded_lot_demand(int N, const vector<int>& demand, const vector<int>& supply,
                          const vector<int>& cost) {
    auto compare = [&](int i, int j) {
        return make_pair(cost[i], i) > make_pair(cost[j], j);
    };

    priority_queue<int, vector<int>, decltype(compare)> suppliers(compare);
    vector<int> prod(N);
    int64_t total = 0;

    for (int i = 0; i < N; i++) {
        int remaining = demand[i];
        suppliers.push(i);
        while (remaining > 0 && !suppliers.empty()) {
            int j = suppliers.top();
            int take = min(supply[j] - prod[j], remaining);
            remaining -= take;
            prod[j] += take;
            total += 1LL * take * cost[j];
            if (prod[j] == supply[j]) {
                suppliers.pop();
            }
        }
        if (remaining > 0) {
            return make_tuple(false, total, move(prod));
        }
    }

    return make_tuple(true, total, move(prod));
}

// We have N jobs to be scheduled. The i-th job has time[i] batches of work.
// The i-th job may be started on day start[i], inclusive.
// The i-th job must be completed by done[i], exclusive.
// The time[i] batches of job i must be completed in succession on different days.
// They may be performed by different machines on non-consecutive days.
// Is there a feasible scheduling? O(N^2), can be optimized with segtree
auto interruptable_parallel_scheduling(int N, int M, const vector<int64_t>& time,
                                       const vector<int>& start,
                                       const vector<int>& done) {
    vector<int> order(N);
    iota(begin(order), end(order), 0);
    sort(begin(order), end(order), [&](int i, int j) {
        return make_tuple(done[i], start[i], i) < make_tuple(done[j], start[j], j);
    });

    vector<int> pts;
    for (int i = 0; i < N; i++) {
        assert(0 <= start[i] && 0 <= time[i] && start[i] + time[i] <= done[i]);
        pts.push_back(start[i]);
        pts.push_back(done[i]);
    }
    sort(begin(pts), end(pts));
    pts.erase(unique(begin(pts), end(pts)), end(pts));
    int P = pts.size() - 1;

    vector<int64_t> spare(P);
    for (int t = 0; t < P; t++) {
        spare[t] = M * (pts[t + 1] - pts[t]);
    }

    for (int i : order) {
        int a = lower_bound(begin(pts), end(pts), start[i]) - begin(pts);
        int b = lower_bound(begin(pts), end(pts), done[i]) - begin(pts);
        auto need = time[i];
        for (int t = a; t < b && need > 0; t++) {
            int64_t take = min<int64_t>({need, spare[t], pts[t + 1] - pts[t]});
            need -= take;
            spare[t] -= take;
        }
        if (need > 0) {
            return false;
        }
    }

    return true;
}
