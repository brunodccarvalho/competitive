#pragma once

#include "matroids.hpp"
#include "struct/integer_heaps.hpp"

/**
 * Non-introspective maximum size matroid intersection with decent heuristics
 * Remember to experiment swapping M1 and M2, as running time can vary significantly
 * for very non-trivial reasons.
 *
 * Complexity (r is the size of the answer):
 *     M1       M2
 *    O(nr)    O(nr)     check()
 *    O(r²)    O(r²)     insert()
 *    O(nr²)   O(nr²)    exchange()
 *    O(r)     O(r)      clear()
 *
 * O(nr²(O1 + O2)) in general.
 */
template <typename Matroid1, typename Matroid2>
auto max_matroid_intersection_11(Matroid1 m1, Matroid2 m2) {
    static_assert(Matroid1::EXCHANGE && Matroid2::EXCHANGE);
    assert(m1.ground_size() == m2.ground_size());

    int n = m1.ground_size();
    vector<bool> in(n);
    vector<int> bfs(n), prev(n), X, Y, Z;

    // Greedy insertion
    for (int u = 0; u < n; u++) {
        if (m2.check(u) && m1.insert_check(u)) {
            m2.insert(u), in[u] = true;
        }
    }

    while (true) {
        int S = 0, sx = 0, sy = 0, sz = 0;
        X.clear(), Y.clear(), Z.clear();

        for (int u = 0; u < n; u++) {
            if (in[u]) {
                X.push_back(u), sx++;
            } else if (m2.check(u)) {
                Y.push_back(u), sy++;
            } else if (m1.check(u)) {
                bfs[S++] = u;
            } else {
                Z.push_back(u), sz++;
            }
        }
        if (S == 0 || sy == 0) {
            break;
        }

        fill(begin(prev), end(prev), -1);
        int finish = -1;

        for (int bi = 0; bi < S; bi++) {
            int u = bfs[bi];
            for (int xi = 0; xi < sx; xi++) {
                if (int x = X[xi]; m2.exchange(x, u)) {
                    prev[x] = u;
                    swap(X[xi--], X[--sx]);
                    for (int y : Y) {
                        if (m1.exchange(x, y)) {
                            prev[y] = x;
                            finish = y;
                            goto augment;
                        }
                    }
                    for (int zi = 0; zi < sz; zi++) {
                        if (int z = Z[zi]; m1.exchange(x, z)) {
                            prev[z] = x;
                            bfs[S++] = z;
                            swap(Z[zi--], Z[--sz]);
                        }
                    }
                }
            }
        }

        break; // no more augmenting paths

    augment:
        while (finish != -1) {
            in[finish].flip();
            finish = prev[finish];
        }

        // Rebuild the matroids
        m1.clear(), m2.clear();
        for (int u = 0; u < n; u++) {
            if (in[u]) {
                m1.insert(u);
                m2.insert(u);
            }
        }
    }

    vector<int> ans;
    for (int u = 0; u < n; u++) {
        if (in[u]) {
            ans.push_back(u);
        }
    }
    return ans;
}

template <typename Matroid1, typename Matroid2>
auto max_matroid_intersection_01(Matroid1 m1, Matroid2 m2) {
    static_assert(!Matroid1::EXCHANGE && Matroid2::EXCHANGE);
    assert(m1.ground_size() == m2.ground_size());

    int n = m1.ground_size();
    vector<bool> in(n);
    vector<int> bfs(n), prev(n), X, Y, Z;

    // Greedy insertion
    for (int u = 0; u < n; u++) {
        if (m2.check(u) && m1.insert_check(u)) {
            m2.insert(u), in[u] = true;
        }
    }

    while (true) {
        int S = 0, sx = 0, sy = 0, sz = 0;
        X.clear(), Y.clear(), Z.clear();

        for (int u = 0; u < n; u++) {
            if (in[u]) {
                X.push_back(u), sx++;
            } else if (m2.check(u)) {
                Y.push_back(u), sy++;
            } else if (m1.check(u)) {
                bfs[S++] = u;
            } else {
                Z.push_back(u), sz++;
            }
        }
        if (S == 0 || sy == 0) {
            break;
        }

        fill(begin(prev), end(prev), -1);
        int finish = -1;

        for (int bi = 0; bi < S; bi++) {
            int u = bfs[bi];
            for (int xi = 0; xi < sx; xi++) {
                if (int x = X[xi]; m2.exchange(x, u)) {
                    prev[x] = u;
                    swap(X[xi--], X[--sx]);
                    m1.clear();
                    for (int other : X) {
                        if (x != other) {
                            m1.insert(other);
                        }
                    }
                    for (int y : Y) {
                        if (m1.check(y)) {
                            prev[y] = x;
                            finish = y;
                            goto augment;
                        }
                    }
                    for (int zi = 0; zi < sz; zi++) {
                        if (int z = Z[zi]; m1.check(z)) {
                            prev[z] = x;
                            bfs[S++] = z;
                            swap(Z[zi--], Z[--sz]);
                        }
                    }
                }
            }
        }

        break; // no more augmenting paths

    augment:
        while (finish != -1) {
            in[finish].flip();
            finish = prev[finish];
        }

        // Rebuild the matroids
        m1.clear(), m2.clear();
        for (int u = 0; u < n; u++) {
            if (in[u]) {
                m1.insert(u);
                m2.insert(u);
            }
        }
    }

    vector<int> ans;
    for (int u = 0; u < n; u++) {
        if (in[u]) {
            ans.push_back(u);
        }
    }
    return ans;
}

/**
 * Find maximum-size minimum cost matroid intersection
 * Remember to experiment swapping M1 and M2, as running time can vary significantly
 * for very non-trivial reasons.
 *
 * Costs must be positive; add a constant to make them so if necessary.
 *
 * Complexity (r is the size of the answer):
 *     M1       M2
 *    Θ(nr)    O(nr)     check()
 *    Θ(r²)    O(r²)     insert()
 *    Θ(rn²)   O(rn²)    exchange()
 *    Θ(r)     O(r)      clear()
 *
 * Θ(rn²(O1 + O2) log n) in general.
 */
template <typename Matroid1, typename Matroid2, typename Cost>
auto mincost_matroid_intersection_11(vector<Cost> cost, Matroid1 m1, Matroid2 m2) {
    static_assert(Matroid1::EXCHANGE && Matroid2::EXCHANGE);
    assert(m1.ground_size() == m2.ground_size());

    static constexpr Cost inf = numeric_limits<Cost>::max();
    int n = m1.ground_size();

    vector<pair<Cost, int>> dist(n);
    vector<Cost> pi(n);
    vector<int> prev(n), X[2];
    vector<bool> in(n), reach(n);
    pairing_int_heap<less_container<vector<pair<Cost, int>>>> heap(n, dist);

    while (true) {
        X[0].clear(), X[1].clear();
        fill(begin(dist), end(dist), make_pair(inf, n + 1));
        fill(begin(prev), end(prev), -1);
        fill(begin(reach), end(reach), false);

        for (int u = 0; u < n; u++) {
            X[in[u]].push_back(u);
        }
        for (int u : X[0]) {
            reach[u] = m2.check(u);
            if (m1.check(u)) {
                dist[u] = {cost[u] - pi[u], 0};
                heap.push(u);
            }
        }

        while (!heap.empty() && !reach[heap.top()]) {
            int u = heap.pop();
            bool b = in[u];

            for (int v : X[!b]) {
                if (b ? m1.exchange(u, v) : m2.exchange(v, u)) {
                    Cost w = cost[v] + pi[u] - pi[v];
                    auto relax = make_pair(dist[u].first + w, dist[u].second + 1);
                    if (dist[v] > relax) {
                        dist[v] = relax;
                        prev[v] = u;
                        heap.push_or_improve(v);
                    }
                }
            }
        }
        if (heap.empty()) {
            break;
        }

        int u = heap.top();
        heap.clear();

        for (int v = 0; v < n; v++) {
            pi[v] += min(dist[v].first, dist[u].first);
        }
        for (int v = u; v != -1; v = prev[v]) {
            in[v].flip();
            pi[v] -= cost[v];
            cost[v] = -cost[v];
        }

        m1.clear(), m2.clear();
        for (int v = 0; v < n; v++) {
            if (in[v]) {
                m1.insert(v);
                m2.insert(v);
            }
        }
    }

    vector<int> ans;
    Cost costsum = 0;
    for (int u = 0; u < n; u++) {
        if (in[u]) {
            ans.push_back(u);
            costsum += cost[u];
        }
    }
    return make_pair(move(ans), -costsum);
}

template <typename Matroid1, typename Matroid2, typename Cost>
auto mincost_matroid_intersection_01(vector<Cost> cost, Matroid1 m1, Matroid2 m2) {
    static_assert(!Matroid1::EXCHANGE && Matroid2::EXCHANGE);
    assert(m1.ground_size() == m2.ground_size());

    static constexpr Cost inf = numeric_limits<Cost>::max();
    int n = m1.ground_size();

    vector<pair<Cost, int>> dist(n);
    vector<Cost> pi(n);
    vector<int> prev(n), X[2];
    vector<bool> in(n), reach(n);
    pairing_int_heap<less_container<vector<pair<Cost, int>>>> heap(n, dist);

    while (true) {
        X[0].clear(), X[1].clear();
        fill(begin(dist), end(dist), make_pair(inf, n + 1));
        fill(begin(prev), end(prev), -1);
        fill(begin(reach), end(reach), false);

        for (int u = 0; u < n; u++) {
            X[in[u]].push_back(u);
        }
        for (int u : X[0]) {
            reach[u] = m2.check(u);
            if (m1.check(u)) {
                dist[u] = {cost[u] - pi[u], 0};
                heap.push(u);
            }
        }

        while (!heap.empty() && !reach[heap.top()]) {
            int u = heap.pop();

            if (bool b = in[u]; b) {
                m1.clear();
                for (int v : X[1]) {
                    if (v != u) {
                        m1.insert(v);
                    }
                }
                for (int v : X[0]) {
                    if (m1.check(v)) {
                        Cost w = cost[v] + pi[u] - pi[v];
                        auto relax = make_pair(dist[u].first + w, dist[u].second + 1);
                        if (dist[v] > relax) {
                            dist[v] = relax;
                            prev[v] = u;
                            heap.push_or_improve(v);
                        }
                    }
                }
            } else {
                for (int v : X[1]) {
                    if (m2.exchange(v, u)) {
                        Cost w = cost[v] + pi[u] - pi[v];
                        auto relax = make_pair(dist[u].first + w, dist[u].second + 1);
                        if (dist[v] > relax) {
                            dist[v] = relax;
                            prev[v] = u;
                            heap.push_or_improve(v);
                        }
                    }
                }
            }
        }
        if (heap.empty()) {
            break;
        }

        int u = heap.top();
        heap.clear();

        for (int v = 0; v < n; v++) {
            pi[v] += min(dist[v].first, dist[u].first);
        }
        for (int v = u; v != -1; v = prev[v]) {
            in[v].flip();
            pi[v] -= cost[v];
            cost[v] = -cost[v];
        }

        m1.clear(), m2.clear();
        for (int v = 0; v < n; v++) {
            if (in[v]) {
                m1.insert(v);
                m2.insert(v);
            }
        }
    }

    vector<int> ans;
    Cost costsum = 0;
    for (int u = 0; u < n; u++) {
        if (in[u]) {
            ans.push_back(u);
            costsum += cost[u];
        }
    }
    return make_pair(move(ans), -costsum);
}

template <typename Matroid1, typename Matroid2, typename Cost>
auto mincost_matroid_intersection_00(vector<Cost> cost, Matroid1 m1, Matroid2 m2) {
    static_assert(!Matroid1::EXCHANGE && !Matroid2::EXCHANGE);
    assert(m1.ground_size() == m2.ground_size());

    static constexpr Cost inf = numeric_limits<Cost>::max();
    int n = m1.ground_size();

    vector<vector<int>> adj(n);
    vector<pair<Cost, int>> dist(n);
    vector<Cost> pi(n);
    vector<int> prev(n), X[2];
    vector<bool> in(n), reach(n);
    pairing_int_heap<less_container<vector<pair<Cost, int>>>> heap(n, dist);

    while (true) {
        X[0].clear(), X[1].clear();
        fill(begin(dist), end(dist), make_pair(inf, n + 1));
        fill(begin(prev), end(prev), -1);
        fill(begin(reach), end(reach), false);

        for (int u = 0; u < n; u++) {
            X[in[u]].push_back(u);
            adj[u].clear();
        }
        for (int u : X[0]) {
            reach[u] = m2.check(u);
            if (m1.check(u)) {
                dist[u] = {cost[u] - pi[u], 0};
                heap.push(u);
            }
        }
        if (heap.empty()) {
            break;
        }

        for (int u : X[1]) {
            m2.clear();
            for (int x : X[1]) {
                if (u != x) {
                    m2.insert(x);
                }
            }
            for (int v : X[0]) {
                if (m2.check(v)) {
                    adj[v].push_back(u);
                }
            }
        }

        while (!heap.empty() && !reach[heap.top()]) {
            int u = heap.pop();

            if (bool b = in[u]; b) {
                m1.clear();
                for (int v : X[1]) {
                    if (v != u) {
                        m1.insert(v);
                    }
                }
                for (int v : X[0]) {
                    if (m1.check(v)) {
                        Cost w = cost[v] + pi[u] - pi[v];
                        auto relax = make_pair(dist[u].first + w, dist[u].second + 1);
                        if (dist[v] > relax) {
                            dist[v] = relax;
                            prev[v] = u;
                            heap.push_or_improve(v);
                        }
                    }
                }
            } else {
                for (int v : adj[u]) {
                    Cost w = cost[v] + pi[u] - pi[v];
                    auto relax = make_pair(dist[u].first + w, dist[u].second + 1);
                    if (dist[v] > relax) {
                        dist[v] = relax;
                        prev[v] = u;
                        heap.push_or_improve(v);
                    }
                }
            }
        }
        if (heap.empty()) {
            break;
        }

        int u = heap.top();
        heap.clear();

        for (int v = 0; v < n; v++) {
            pi[v] += min(dist[v].first, dist[u].first);
        }
        for (int v = u; v != -1; v = prev[v]) {
            in[v].flip();
            pi[v] -= cost[v];
            cost[v] = -cost[v];
        }

        m1.clear(), m2.clear();
        for (int v = 0; v < n; v++) {
            if (in[v]) {
                m1.insert(v);
                m2.insert(v);
            }
        }
    }

    vector<int> ans;
    Cost costsum = 0;
    for (int u = 0; u < n; u++) {
        if (in[u]) {
            ans.push_back(u);
            costsum += cost[u];
        }
    }
    return make_pair(move(ans), -costsum);
}
