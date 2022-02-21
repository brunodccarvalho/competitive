#pragma once

#include "matroids.hpp"
#include "struct/integer_heaps.hpp"
#include "struct/circular_queue.hpp"

/**
 * Find a minimum-size partitioning of the ground set elements into pairwise disjoint
 * independent sets.
 *
 * Complexity, where k is the answer:
 *    O(kn²)   check()
 *    O(n²)    insert()
 *    O(n³)    exchange()
 *    O(nk)    clear()
 *
 * O(n³) in general, but heuristics make it close to O(nk) usually.
 */
template <typename Matroid>
auto min_matroid_partitioning_1(Matroid matroid) {
    static_assert(Matroid::EXCHANGE);
    int n = matroid.ground_size();

    vector<int> active, bfs(n), prev(n), in(n, -1), count(1);
    vector<Matroid> sets = {matroid};
    int k = 1; // how many sets currently

    static mt19937 rng(random_device{}());
    vector<int> elems(n);
    iota(begin(elems), end(elems), 0);
    shuffle(begin(elems), end(elems), rng);

    // Greedily turn sets[0] into a basis, then add a minimum number of sets
    for (int u : elems) {
        if (sets[0].insert_check(u)) {
            active.push_back(u);
            in[u] = 0, count[0]++;
        }
    }

    // We will need at least ceil(n/A) sets, so just greedily fill up those many
    int A = count[0];
    k = (n + A - 1) / A;
    for (int i = 1; i < k; i++) {
        sets.emplace_back(matroid);
        count.push_back(0);
    }

    for (int x : elems) {
        // Greedily try to fit x into any of the current sets, try most recent first
        for (int s = k - 1; s >= 0 && in[x] == -1; s--) {
            if (count[s] < A && sets[s].insert_check(x)) {
                active.push_back(x);
                in[x] = s, count[s]++;
                break;
            }
        }
        if (in[x] != -1 || !matroid.check(x)) {
            continue;
        }

        fill(begin(prev), end(prev), -1);
        int S = 0;

        // Start the bfs from the sets themselves
        for (int s = 0; s < k && prev[x] == -1; s++) {
            if (count[s] < A) {
                for (int v : active) {
                    if (prev[v] == -1 && in[v] != s && sets[s].check(v)) {
                        prev[v] = n + s;
                        bfs[S++] = v;
                        if (int t = in[v]; sets[t].exchange(v, x)) {
                            prev[x] = v;
                            break;
                        }
                    }
                }
            }
        }

        // Continue through the elements if necessary
        for (int i = 0; i < S && prev[x] == -1; i++) {
            int u = bfs[i], s = in[u];
            for (int v : active) {
                if (prev[v] == -1 && in[v] != s && sets[s].exchange(u, v)) {
                    prev[v] = u;
                    bfs[S++] = v;
                    if (int t = in[v]; sets[t].exchange(v, x)) {
                        prev[x] = v;
                        break;
                    }
                }
            }
        }

        active.push_back(x);

        // Add a new set with just x
        if (prev[x] == -1) {
            sets.push_back(matroid);
            sets[k++].insert(x);
            in[x] = k, count.push_back(1);
            continue;
        }

        // Rebuild all the affected sets
        vector<bool> affected(k);
        while (prev[x] < n) {
            in[x] = in[prev[x]];
            affected[in[x]] = true;
            x = prev[x];
        }
        int root = prev[x] - n;
        in[x] = root, count[root]++;
        affected[root] = true;

        for (int s = 0; s < k; s++) {
            if (affected[s]) {
                sets[s].clear();
            }
        }
        for (int u : active) {
            if (affected[in[u]]) {
                sets[in[u]].insert(u);
            }
        }
    }

    int used = active.size();
    return make_tuple(used, k, move(in));
}

/**
 * Find a perfect partitioning of the ground set elements into independent sets, one
 * for each of the given matroids.
 *
 * For better performance, put faster matroids first.
 *
 * Complexity, where k is the number of matroids:
 *    O(nk²)  check()
 *    O(nk²)  insert()
 *    O(n³)   exchange()
 *    O(nk)    clear()
 *
 * O(n³ + kn²) in general.
 */
template <typename Matroid>
auto multi_matroid_partitioning_1(vector<Matroid> matroids) {
    static_assert(Matroid::EXCHANGE);
    int k = matroids.size();
    int n = matroids[0].ground_size();

    vector<int> active, bfs(n), prev(n), in(n, -1);

    static mt19937 rng(random_device{}());
    vector<int> elems(n);
    iota(begin(elems), end(elems), 0);
    shuffle(begin(elems), end(elems), rng);

    for (int x : elems) {
        for (int s = 0; s < k; s++) {
            if (matroids[s].insert_check(x)) {
                active.push_back(x);
                in[x] = s;
                break;
            }
        }
        if (in[x] != -1) {
            continue;
        }

        fill(begin(prev), end(prev), -1);
        int S = 0;

        for (int s = 0; s < k && prev[x] == -1; s++) {
            for (int v : active) {
                if (prev[v] == -1 && in[v] != s && matroids[s].check(v)) {
                    prev[v] = n + s;
                    bfs[S++] = v;
                    if (int t = in[v]; matroids[t].exchange(v, x)) {
                        prev[x] = v;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < S && prev[x] == -1; i++) {
            int u = bfs[i], s = in[u];
            for (int v : active) {
                if (prev[v] == -1 && in[v] != s && matroids[s].exchange(u, v)) {
                    prev[v] = u;
                    bfs[S++] = v;
                    if (int t = in[v]; matroids[t].exchange(v, x)) {
                        prev[x] = v;
                        break;
                    }
                }
            }
        }

        if (prev[x] == -1) {
            return make_pair(false, move(in));
        }

        active.push_back(x);

        // Augment along the path, keep track of affected oracles
        vector<bool> affected(k);
        while (prev[x] < n) {
            in[x] = in[prev[x]];
            affected[in[x]] = true;
            x = prev[x];
        }
        int root = prev[x] - n;
        in[x] = root;
        affected[root] = true;

        // Rebuild the affected matroid oracles
        for (int s = 0; s < k; s++) {
            if (affected[s]) {
                matroids[s].clear();
            }
        }
        for (int u : active) {
            if (affected[in[u]]) {
                matroids[in[u]].insert(u);
            }
        }
    }

    return make_pair(true, move(in));
}

/**
 * Find a perfect partitioning of the elements of the ground set into independent sets,
 * one for each of the given matroids, such that set i has size in range between lower[i]
 * and upper[i].
 *
 * For better performance, put faster matroids first.
 *
 * Complexity, where k is the number of matroids:
 *    O(kn²)  check()
 *    O(kn²)  insert()
 *    O(n³)   exchange()
 *    O(nk)    clear()
 *
 * Complexity: O(n³ + kn²) in general if oracle operations amortize to O(1).
 * Usually much better.
 */
template <typename Matroid>
auto restricted_matroid_partitioning_1(vector<Matroid> matroids, const vector<int>& lower,
                                       const vector<int>& upper) {
    static_assert(Matroid::EXCHANGE);
    int k = matroids.size();
    int n = matroids[0].ground_size();

    auto minimum = accumulate(begin(lower), end(lower), 0LL);
    auto maximum = accumulate(begin(upper), end(upper), 0LL);
    assert(matroids.size() == lower.size() && matroids.size() == upper.size());
    assert(minimum <= n && n <= maximum);

    vector<int> active, bfs(n), prev(n), count(k), in(n, -1);

    // First grow sets up to size lower[s]. Then grow them up to size upper[s].
    auto within_bounds = [&](int s) {
        int A = active.size();
        return A < minimum ? count[s] < lower[s] : count[s] < upper[s];
    };

    static mt19937 rng(random_device{}());
    vector<int> elems(n);
    iota(begin(elems), end(elems), 0);
    shuffle(begin(elems), end(elems), rng);

    for (int x : elems) {
        for (int s = 0; s < k; s++) {
            if (within_bounds(s) && matroids[s].insert_check(x)) {
                active.push_back(x);
                in[x] = s, count[s]++;
                break;
            }
        }
        if (in[x] != -1) {
            continue;
        }

        fill(begin(prev), end(prev), -1);
        int S = 0;

        for (int s = 0; s < k && prev[x] == -1; s++) {
            if (within_bounds(s)) {
                for (int v : active) {
                    if (prev[v] == -1 && in[v] != s && matroids[s].check(v)) {
                        prev[v] = n + s;
                        bfs[S++] = v;
                        if (int t = in[v]; matroids[t].exchange(v, x)) {
                            prev[x] = v;
                            break;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < S && prev[x] == -1; i++) {
            int u = bfs[i], s = in[u];
            for (int v : active) {
                if (prev[v] == -1 && in[v] != s && matroids[s].exchange(u, v)) {
                    prev[v] = u;
                    bfs[S++] = v;
                    if (int t = in[v]; matroids[t].exchange(v, x)) {
                        prev[x] = v;
                        break;
                    }
                }
            }
        }

        // Bail if we failed to find a path, and thus there is no perfect partitioning
        if (prev[x] == -1) {
            return make_pair(false, move(in));
        }

        active.push_back(x);

        // Augment along the path, keep track of affected oracles
        vector<bool> affected(k);
        while (prev[x] < n) {
            in[x] = in[prev[x]];
            affected[in[x]] = true;
            x = prev[x];
        }
        int root = prev[x] - n;
        in[x] = root, count[root]++;
        affected[root] = true;

        // Rebuild the affected matroid oracles
        for (int s = 0; s < k; s++) {
            if (affected[s]) {
                matroids[s].clear();
            }
        }
        for (int u : active) {
            if (affected[in[u]]) {
                matroids[in[u]].insert(u);
            }
        }
    }

    return make_pair(true, move(in));
}

/**
 * min (sum, sum) partitioning
 *
 * Find the minimum cost, maximum-size partitioning of the ground set among k matroids,
 * not necessarily identical (requires pairing_int_heap)
 *
 * cost[k][i]: Cost of placing ground set element #i into independent set #k.
 * Costs must be positive; add a constant to make them so if necessary.
 *
 * Complexity, where k is the number of matroids and m is the size of the answer:
 *    Θ(mnk)  check()
 *    Θ(mnk)  insert()
 *    Θ(mn²)  exchange()
 *    Θ(nk)   clear()
 *
 * Θ(mn(n+k)Oi) in general. Note that the answer may not be correct if m<n.
 */
template <typename Matroid, typename Cost>
auto mincost_matroid_partitioning_1(const vector<vector<Cost>>& cost,
                                    vector<Matroid> matroids) {
    static_assert(Matroid::EXCHANGE);

    static constexpr Cost inf = numeric_limits<Cost>::max();
    int k = matroids.size();
    int n = matroids[0].ground_size();

    vector<pair<Cost, int>> dist(n);
    vector<Cost> pi(n);
    vector<int> prev(n), in(n, -1);
    pairing_int_heap<less_container<vector<pair<Cost, int>>>> heap(n, dist);

    auto transition = [&](int u, int s) {
        return cost[s][u] - (in[u] == -1 ? 0 : cost[in[u]][u]);
    };

    for (int run = 0; run < n; run++) {
        fill(begin(dist), end(dist), make_pair(inf, n));
        fill(begin(prev), end(prev), -1);

        // First edge: for every set s, check insertion of elements from other sets into s
        for (int v = 0; v < n; v++) {
            for (int s = 0; s < k; s++) {
                if (in[v] != s) {
                    Cost w = transition(v, s) - pi[v];
                    auto relax = make_pair(w, 0);
                    if (dist[v] > relax && matroids[s].check(v)) {
                        dist[v] = relax;
                        prev[v] = s + n;
                    }
                }
            }
            if (prev[v] != -1) {
                heap.push(v);
            }
        }

        while (!heap.empty() && in[heap.top()] != -1) {
            int u = heap.pop(), s = in[u];

            for (int v = 0; v < n; v++) {
                if (in[v] != s) {
                    Cost w = transition(v, s) + pi[u] - pi[v];
                    auto relax = make_pair(dist[u].first + w, dist[u].second + 1);
                    if (dist[v] > relax && matroids[s].exchange(u, v)) {
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

        // Single edge augmentation
        if (prev[u] >= n) {
            int root = prev[u] - n;
            pi[u] -= transition(u, root);
            in[u] = root;
            matroids[root].insert(u);
            continue;
        }

        // Augment along the path, keep track of affected oracles
        vector<bool> affected(k);
        while (prev[u] < n) {
            pi[u] -= transition(u, in[prev[u]]);
            in[u] = in[prev[u]];
            affected[in[u]] = true;
            u = prev[u];
        }
        int root = prev[u] - n;
        pi[u] -= transition(u, root);
        in[u] = root;
        affected[root] = true;

        // Rebuild the affected matroid oracles
        for (int s = 0; s < k; s++) {
            if (affected[s]) {
                matroids[s].clear();
            }
        }
        for (int v = 0; v < n; v++) {
            if (in[v] != -1 && affected[in[v]]) {
                matroids[in[v]].insert(v);
            }
        }
    }

    int A = 0;
    Cost costsum = 0;
    for (int u = 0; u < n; u++) {
        if (int s = in[u]; s >= 0) {
            A++, costsum += cost[s][u];
        }
    }
    return make_tuple(A, move(in), costsum);
}

template <typename Matroid, typename Cost>
auto mincost_matroid_partitioning_0(const vector<vector<Cost>>& cost,
                                    vector<Matroid> matroids) {
    static_assert(!Matroid::EXCHANGE);

    static constexpr Cost inf = numeric_limits<Cost>::max();
    int k = matroids.size();
    int n = matroids[0].ground_size();

    vector<vector<int>> onset(k);
    vector<pair<Cost, int>> dist(n);
    vector<Cost> pi(n);
    vector<int> prev(n), in(n, -1);
    pairing_int_heap<less_container<vector<pair<Cost, int>>>> heap(n, dist);

    auto transition = [&](int u, int s) {
        return cost[s][u] - (in[u] == -1 ? 0 : cost[in[u]][u]);
    };

    for (int run = 0; run < n; run++) {
        fill(begin(dist), end(dist), make_pair(inf, n));
        fill(begin(prev), end(prev), -1);

        // First edge: for every set s, check insertion of elements from other sets into s
        for (int v = 0; v < n; v++) {
            for (int s = 0; s < k; s++) {
                if (in[v] != s) {
                    Cost w = transition(v, s) - pi[v];
                    auto relax = make_pair(w, 0);
                    if (dist[v] > relax && matroids[s].check(v)) {
                        dist[v] = relax;
                        prev[v] = s + n;
                    }
                }
            }
            if (prev[v] != -1) {
                heap.push(v);
            }
        }

        while (!heap.empty() && in[heap.top()] != -1) {
            int u = heap.pop(), s = in[u];

            matroids[s].clear();
            for (int v : onset[s]) {
                if (u != v) {
                    matroids[s].insert(v);
                }
            }

            for (int v = 0; v < n; v++) {
                if (in[v] != s) {
                    Cost w = transition(v, s) + pi[u] - pi[v];
                    auto relax = make_pair(dist[u].first + w, dist[u].second + 1);
                    if (dist[v] > relax && matroids[s].check(v)) {
                        dist[v] = relax;
                        prev[v] = u;
                        heap.push_or_improve(v);
                    }
                }
            }

            matroids[s].insert(u);
        }
        if (heap.empty()) {
            break;
        }

        int u = heap.top();
        heap.clear();

        for (int v = 0; v < n; v++) {
            pi[v] += min(dist[v].first, dist[u].first);
        }

        // Single edge augmentation
        if (prev[u] >= n) {
            int root = prev[u] - n;
            pi[u] -= transition(u, root);
            in[u] = root;
            matroids[root].insert(u);
            onset[root].push_back(u);
            continue;
        }

        // Augment along the path, keep track of affected oracles
        vector<bool> affected(k);
        while (prev[u] < n) {
            pi[u] -= transition(u, in[prev[u]]);
            in[u] = in[prev[u]];
            affected[in[u]] = true;
            u = prev[u];
        }
        int root = prev[u] - n;
        pi[u] -= transition(u, root);
        in[u] = root;
        affected[root] = true;

        // Rebuild the affected matroid oracles
        for (int s = 0; s < k; s++) {
            if (affected[s]) {
                matroids[s].clear();
                onset[s].clear();
            }
        }
        for (int v = 0; v < n; v++) {
            if (in[v] != -1 && affected[in[v]]) {
                matroids[in[v]].insert(v);
                onset[in[v]].push_back(v);
            }
        }
    }

    int A = 0;
    Cost costsum = 0;
    for (int u = 0; u < n; u++) {
        if (int s = in[u]; s >= 0) {
            A++, costsum += cost[s][u];
        }
    }
    return make_tuple(A, move(in), costsum);
}

template <typename Matroid, typename Cost>
auto mincost_matroid_partitioning_1_spfa(const vector<vector<Cost>>& cost,
                                         vector<Matroid> matroids) {
    static_assert(Matroid::EXCHANGE);

    static constexpr Cost inf = numeric_limits<Cost>::max();
    int k = matroids.size();
    int n = matroids[0].ground_size();

    vector<pair<Cost, int>> dist(n + 1);
    vector<int> active, prev(n + 1), in(n, -1);
    spfa_deque<less_container<vector<pair<Cost, int>>>> spfa(n, dist);

    static mt19937 rng(random_device{}());
    vector<int> elems(n);
    iota(begin(elems), end(elems), 0);
    shuffle(begin(elems), end(elems), rng);

    auto transition = [&](int u, int s) {
        return cost[s][u] - (in[u] == -1 ? 0 : cost[in[u]][u]);
    };

    for (int run = 0; run < n; run++) {
        fill(begin(dist), end(dist), make_pair(inf, n));
        fill(begin(prev), end(prev), -1);

        // First edge: for every set s, check insertion of elements from other sets into s
        for (int s = 0; s < k; s++) {
            for (int v : elems) {
                if (in[v] != s) {
                    Cost w = transition(v, s);
                    auto relax = make_pair(w, 0);
                    if (dist[v] > relax && matroids[s].check(v)) {
                        dist[v] = relax;
                        prev[v] = s + n;
                        if (in[v] != -1) {
                            spfa.push(v);
                        }
                        if (in[v] == -1 && dist[n] > dist[v]) {
                            dist[n] = dist[v];
                            prev[n] = v;
                        }
                    }
                }
            }
        }

        // Run SPFA to extend paths.
        // Push only active elements onto the queue; update n on inactive relaxations.
        while (!spfa.empty()) {
            int u = spfa.pop(), s = in[u];

            for (int v : elems) {
                if (in[v] != s) {
                    Cost w = transition(v, s);
                    auto relax = make_pair(dist[u].first + w, dist[u].second + 1);
                    if (dist[v] > relax && matroids[s].exchange(u, v)) {
                        dist[v] = relax;
                        prev[v] = u;
                        if (in[v] != -1) {
                            spfa.push(v);
                        }
                        if (in[v] == -1 && dist[n] > dist[v]) {
                            dist[n] = dist[v];
                            prev[n] = v;
                        }
                    }
                }
            }
        }

        // Bail if we failed to find a path, and thus there is no further augmentation
        if (prev[n] == -1) {
            break;
        }

        int u = prev[n];
        active.push_back(u);

        // Single edge augmentation
        if (prev[u] >= n) {
            int root = prev[u] - n;
            in[u] = root;
            matroids[root].insert(u);
            continue;
        }

        // Augment along the path, keep track of affected oracles
        vector<bool> affected(k);
        while (prev[u] < n) {
            in[u] = in[prev[u]];
            affected[in[u]] = true;
            u = prev[u];
        }
        int root = prev[u] - n;
        in[u] = root;
        affected[root] = true;

        // Rebuild the affected matroid oracles
        for (int s = 0; s < k; s++) {
            if (affected[s]) {
                matroids[s].clear();
            }
        }
        for (int v : active) {
            if (affected[in[v]]) {
                matroids[in[v]].insert(v);
            }
        }
    }

    int A = 0;
    Cost costsum = 0;
    for (int u = 0; u < n; u++) {
        if (int s = in[u]; s >= 0) {
            A++, costsum += cost[s][u];
        }
    }
    return make_tuple(A, move(in), costsum);
}

/**
 * min (max, max) partitioning
 *
 * Find the partitioning of the ground set elements among k matroids which minimizes the
 * largest overall cost.
 *
 * cost[k][i]: Cost of placing ground set element #i into independent set #k.
 *
 * Complexity, where k is the number of matroids:
 *    O(nk² log maxw)  check()
 *    O(nk² log maxw)  insert()
 *    O(n³ log maxw)   exchange()
 *    O(nk log maxw)   clear()
 *
 * O((n³ + kn²) log maxw) in general.
 */
template <typename Matroid, typename Cost>
auto min_maxcost_matroid_partitioning_1(const vector<vector<Cost>>& cost,
                                        vector<Matroid> matroids) {
    static_assert(Matroid::EXCHANGE);

    int k = matroids.size();
    int n = matroids[0].ground_size();

    Cost mincost = numeric_limits<Cost>::max();
    Cost maxcost = numeric_limits<Cost>::lowest();

    for (int s = 0; s < k; s++) {
        for (int i = 0; i < n; i++) {
            mincost = min(mincost, cost[s][i]);
            maxcost = max(maxcost, cost[s][i]);
        }
    }

    vector<cost_cutoff_matroid<Matroid, Cost>> ms;
    ms.reserve(k);
    for (int s = 0; s < k; s++) {
        ms.emplace_back(matroids[s], cost[s], 0);
    }

    Cost L = mincost - 1, R = maxcost + 1;
    auto best = make_tuple(false, vector<int>(n, -1), Cost(-1));

    while (L + 1 < R) {
        Cost M = L + (R - L) / 2;
        for (int s = 0; s < k; s++) {
            ms[s].cut = M;
        }

        auto [ok, in] = multi_matroid_partitioning_1(ms);

        if (ok) {
            best = make_tuple(true, move(in), M);
            R = M;
        } else {
            L = M;
        }
    }

    return best;
}
