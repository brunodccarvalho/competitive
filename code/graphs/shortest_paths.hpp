#pragma once

#include "struct/pbds.hpp"
#include "struct/integer_heaps.hpp"
#include "struct/circular_queue.hpp"

template <typename Cost = long, typename CostSum = Cost>
auto spfa(int s, const vector<vector<pair<int, Cost>>>& adj) {
    constexpr CostSum inf = numeric_limits<CostSum>::max() / 2;

    int V = adj.size();
    vector<CostSum> dist(V, inf);
    // vector<int> prev(V, -1);
    dist[s] = 0;

    // vector<int> visits(V, 0);
    spfa_deque<less_container<vector<CostSum>>> Q(V, dist);
    Q.push(s);

    do {
        int u = Q.pop();

        // if visits[u]>V, negative cycle detected
        // ++visits[u], assert(visits[u] <= V);

        for (auto [v, w] : adj[u]) {
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                // prev[v] = u;
                Q.push(v);
            }
        }
    } while (!Q.empty());

    // return make_pair(move(dist), move(prev));
    return dist;
}

template <typename Cost = long, typename CostSum = Cost>
auto dijkstra(int s, const vector<vector<pair<int, Cost>>>& adj) {
    constexpr CostSum inf = numeric_limits<CostSum>::max() / 2;

    int V = adj.size();
    vector<CostSum> dist(V, inf);
    // vector<int> prev(V, -1);
    dist[s] = 0;

    pairing_int_heap<less_container<vector<CostSum>>> heap(V, dist);
    heap.push(s);

    do {
        int u = heap.pop();
        for (auto [v, w] : adj[u]) {
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                // prev[v] = u;
                heap.push_or_improve(v);
            }
        }
    } while (!heap.empty());

    // return make_pair(move(dist), move(prev));
    return dist;
}

template <typename Cost = long, typename CostSum = Cost>
auto bellman_ford(int V, int s, const vector<tuple<int, int, Cost>>& edge) {
    constexpr CostSum inf = numeric_limits<CostSum>::max() / 2;

    vector<CostSum> dist(V, inf);
    // vector<int> prev(V, -1);
    dist[s] = 0;

    bool stop = false;
    for (int phase = 0; phase < V && !stop; phase++) {
        stop = true;
        for (auto [u, v, w] : edge) {
            if (dist[v] > dist[u] + w && dist[u] < inf) {
                dist[v] = dist[u] + w;
                // prev[v] = u;
                stop = false;
            }
        }
    }
    // if stop is false, negative cycle detected
    assert(stop);

    // return make_pair(move(dist), move(prev));
    return dist;
}

template <typename Cost = long, typename CostSum = Cost>
auto bellman_ford(int s, const vector<vector<pair<int, Cost>>>& adj) {
    constexpr CostSum inf = numeric_limits<CostSum>::max() / 2;

    int V = adj.size();
    vector<CostSum> dist(V, inf);
    // vector<int> prev(V, -1);
    dist[s] = 0;

    bool stop = false;
    for (int phase = 0; phase < V && !stop; phase++) {
        stop = true;
        for (int u = 0; u < V; u++) {
            for (auto [v, w] : adj[u]) {
                if (dist[v] > dist[u] + w && dist[u] < inf) {
                    dist[v] = dist[u] + w;
                    // prev[v] = u;
                    stop = false;
                }
            }
        }
    }
    // if stop is false, negative cycle detected
    assert(stop);

    // return make_pair(move(dist), move(prev));
    return dist;
}

template <typename Cost = long, typename CostSum = Cost>
bool bellman_ford_check(vector<vector<pair<int, Cost>>>& adj) {
    constexpr CostSum inf = numeric_limits<CostSum>::max() / 2;

    int V = adj.size();
    vector<CostSum> dist(V + 1, inf);
    dist[V] = 0;

    adj.emplace_back(V);
    for (int v = 0; v < V; v++) {
        adj[V][v] = {v, 0};
    }

    bool stop = false;
    for (int phase = 0; phase <= V && !stop; phase++) {
        stop = true;
        for (int u = 0; u <= V; u++) {
            for (auto [v, w] : adj[u]) {
                if (dist[v] > dist[u] + w && dist[u] < inf) {
                    dist[v] = dist[u] + w;
                    stop = false;
                }
            }
        }
    }

    adj.pop_back();

    // if stop is false, negative cycle detected
    return stop;
}

template <typename Cost = long, typename CostSum = Cost>
auto floyd_warshall(int V, const vector<tuple<int, int, Cost>>& edge) {
    constexpr CostSum inf = numeric_limits<CostSum>::max() / 2;

    vector<vector<CostSum>> dist(V, vector<CostSum>(V, inf));
    // vector<vector<int>> next(V, vector<int>(V, -1));

    for (auto [u, v, w] : edge) {
        dist[u][v] = min(w, dist[u][v]);
        // next[u][v] = v;
    }
    for (int u = 0; u < V; u++) {
        dist[u][u] = 0;
        // next[u][u] = u;
    }

    for (int k = 0; k < V; k++) {
        for (int u = 0; u < V; u++) {
            for (int v = 0; v < V; v++) {
                if (dist[u][v] > dist[u][k] + dist[k][v]) {
                    dist[u][v] = dist[u][k] + dist[k][v];
                    // next[u][v] = next[u][k];
                }
            }
            // if dist[u][u] < 0, negative cycle detected
            assert(dist[u][u] == 0);
        }
    }

    // fix infinites getting reduced by negative cost edges
    for (int u = 0; u < V; u++) {
        for (int v = 0; v < V; v++) {
            if (dist[u][v] > inf / 2) {
                dist[u][v] = inf;
                // next[u][v] = -1;
            }
        }
    }

    // return make_pair(move(dist), move(next));
    return dist;
}

template <typename Cost = long, typename CostSum = Cost>
auto floyd_warshall(const vector<vector<pair<int, Cost>>>& adj) {
    constexpr CostSum inf = numeric_limits<CostSum>::max() / 2;

    int V = adj.size();
    vector<vector<CostSum>> dist(V, vector<CostSum>(V, inf));
    // vector<vector<int>> next(V, vector<int>(V, -1));

    for (int u = 0; u < V; u++) {
        for (auto [v, w] : adj[u]) {
            dist[u][v] = min(w, dist[u][v]);
            // next[u][v] = v;
        }
        dist[u][u] = 0;
        // next[u][u] = u;
    }

    for (int k = 0; k < V; k++) {
        for (int u = 0; u < V; u++) {
            for (int v = 0; v < V; v++) {
                if (dist[u][v] > dist[u][k] + dist[k][v]) {
                    dist[u][v] = dist[u][k] + dist[k][v];
                    // next[u][v] = next[u][k];
                }
            }
            // if dist[u][u] < 0, negative cycle detected
            assert(dist[u][u] == 0);
        }
    }

    // fix infinites getting reduced by negative cost edges
    for (int u = 0; u < V; u++) {
        for (int v = 0; v < V; v++) {
            if (dist[u][v] > inf / 2) {
                dist[u][v] = inf;
                // next[u][v] = -1;
            }
        }
    }

    // return make_pair(move(dist), move(next));
    return dist;
}

template <typename Cost = long, typename CostSum = Cost>
auto dijkstra_all(const vector<vector<pair<int, Cost>>>& adj) {
    constexpr CostSum inf = numeric_limits<CostSum>::max() / 2;

    int V = adj.size();
    vector<vector<CostSum>> dist(V, vector<CostSum>(V, inf));
    // vector<vector<int>> prev(V, vector<int>(V, -1));

    int s = 0;
    auto cmp = [&](int u, int v) { return dist[s][u] < dist[s][v]; };
    pairing_int_heap<decltype(cmp)> heap(V, cmp);

    while (s < V) {
        dist[s][s] = 0;
        heap.push(s);

        do {
            int u = heap.pop();
            for (auto [v, w] : adj[u]) {
                if (dist[s][v] > dist[s][u] + w) {
                    dist[s][v] = dist[s][u] + w;
                    // prev[s][v] = u;
                    heap.push_or_improve(v);
                }
            }
        } while (!heap.empty());

        s++;
    }

    // return make_pair(move(dist), move(prev));
    return dist;
}

template <typename Cost = long, typename CostSum = Cost>
auto johnsons(vector<vector<pair<int, Cost>>>& adj) {
    constexpr CostSum inf = numeric_limits<CostSum>::max() / 2;

    int V = adj.size();
    vector<CostSum> pi(V, 0);
    vector<vector<CostSum>> dist(V, vector<CostSum>(V, inf));
    // vector<vector<int>> next(V, vector<int>(V, -1));

    // Step 1: spfa starting on extra node V to compute potentials
    // vector<int> visits(V, 0);
    spfa_deque<less_container<vector<CostSum>>> spfa(V, pi);
    for (int u = 0; u < V; u++) {
        spfa.push(u);
    }

    do {
        int u = spfa.pop();

        // if visits[u]>V, negative cycle detected
        // ++visits[u], assert(visits[u] <= V);

        for (auto [v, w] : adj[u]) {
            if (pi[v] > pi[u] + w) {
                pi[v] = pi[u] + w;
                spfa.push(v);
            }
        }
    } while (!spfa.empty());

    // Step 2: run dijkstra V times; removed extra edges above
    int s = 0;
    auto cmp = [&](int u, int v) { return dist[s][u] < dist[s][v]; };
    pairing_int_heap<decltype(cmp)> heap(V, cmp);

    while (s < V) {
        dist[s][s] = 0;
        heap.push(s);

        do {
            int u = heap.pop();
            for (auto [v, w] : adj[u]) {
                if (dist[s][v] > dist[s][u] + pi[u] - pi[v] + w) {
                    dist[s][v] = dist[s][u] + pi[u] - pi[v] + w;
                    // prev[s][v] = u;
                    heap.push_or_improve(v);
                }
            }
        } while (!heap.empty());

        for (int u = 0; u < V; u++) {
            if (dist[s][u] != inf) {
                dist[s][u] += pi[u] - pi[s];
            }
        }
        s++;
    }

    // return make_pair(move(dist), move(prev));
    return dist;
}

template <typename Cost = long, typename CostSum = Cost, typename Fn>
auto astar(int s, int t, const vector<vector<pair<int, Cost>>>& adj, Fn&& heuristic) {
    constexpr CostSum inf = numeric_limits<CostSum>::max() / 2;

    int V = adj.size();
    vector<CostSum> dist(V, inf), heur(V, inf);
    // vector<int> prev(V, -1);
    dist[s] = heur[s] = 0;

    pairing_int_heap<less_container<vector<CostSum>>> heap(V, heur);
    heap.push(s);

    do {
        int u = heap.pop();
        for (auto [v, w] : adj[u]) {
            if (dist[v] > dist[u] + w) {
                dist[v] = dist[u] + w;
                heur[v] = dist[v] + heuristic(v);
                // prev[v] = u;
                heap.push_or_improve(v);
            }
        }
    } while (!heap.empty());

    return dist[t];
}
