#pragma once

#include "algo/y_combinator.hpp"

auto build_euler_tour_tree_index(const vector<vector<int>>& tree, int root) {
    int V = tree.size();
    vector<array<int, 2>> index(V);
    int timer = 0;

    auto dfs = y_combinator([&](auto self, int u, int p) -> void {
        index[u][0] = timer++;
        for (int v : tree[u]) {
            if (v != p) {
                self(v, u);
            }
        }
        index[u][1] = timer++;
    });

    dfs(root, -1);
    return index;
}

auto find_tree_centroids(const vector<vector<int>>& tree) {
    int V = tree.size();
    vector<int> subsize(V), centroids;

    auto dfs_centroid = y_combinator([&](auto self, int u, int p) -> void {
        subsize[u] = 1;
        bool is = true;
        for (int v : tree[u]) {
            if (v != p) {
                self(v, u);
                subsize[u] += subsize[v];
                is &= subsize[v] <= V / 2;
            }
        }
        if (is && V - subsize[u] <= V / 2)
            centroids.push_back(u);
    });

    dfs_centroid(0, -1);
    assert(1u <= centroids.size() && centroids.size() <= 2u);
    return centroids;
}

auto find_tree_centers(const vector<vector<int>>& tree) {
    int V = tree.size();
    vector<int> degree(V), dist(V, 0), bfs(V);
    int i = 0, S = 0;

    for (int u = 0; u < V; u++) {
        degree[u] = tree[u].size();
        assert(degree[u] >= 1);
        if (degree[u] == 1) {
            bfs[S++] = u;
        }
    }

    while (i < S) {
        int u = bfs[i++];
        for (int v : tree[u]) {
            if (--degree[v] == 1) {
                bfs[S++] = v;
                dist[v] = dist[u] + 1;
            }
        }
    }
    // up to 2 centers.
    assert(S == V && (V < 3 || dist[bfs[V - 3]] != dist[bfs[V - 1]]));

    vector<int> centers;
    if (V > 0) {
        centers.push_back(bfs[V - 1]);
    }
    if (V > 1 && dist[bfs[V - 1]] == dist[bfs[V - 2]]) {
        centers.push_back(bfs[V - 2]);
    }

    // The diameter length can be found from dist[centers[..]]
    return centers;
}

auto find_tree_diameter(const vector<vector<int>>& tree) {
    int V = tree.size();
    vector<int> degree(V), dist(V, 0), bfs(V), prev(V, -1);
    int i = 0, S = 0;

    for (int u = 0; u < V; u++) {
        degree[u] = tree[u].size();
        if (degree[u] == 1) {
            bfs[S++] = u;
        }
    }

    while (i < S) {
        int u = bfs[i++];
        for (int v : tree[u]) {
            if (--degree[v] == 1) {
                bfs[S++] = v;
                dist[v] = dist[u] + 1;
                prev[v] = u;
            }
        }
    }
    // up to 2 centers.
    assert((V == 1 || S == V) && (V < 3 || dist[bfs[V - 3]] != dist[bfs[V - 1]]));

    vector<int> diameter;

    if (V > 0) {
        int u = bfs[V - 1];
        do {
            diameter.push_back(u);
            u = prev[u];
        } while (u != -1);
    }
    if (V > 1) {
        int u = bfs[V - 2];
        reverse(begin(diameter), end(diameter));
        do {
            diameter.push_back(u);
            u = prev[u];
        } while (u != -1);
    }

    return diameter;
}

auto is_bipartite(int V, const vector<vector<int>>& adj) {
    vector<int8_t> side(V, -1);
    vector<int> bfs;
    int i = 0, S = 0;

    for (int s = 0; s < V; s++) {
        if (side[s] == -1) {
            bfs.push_back(s), S++;
            side[s] = 0;

            while (i < S) {
                int u = bfs[i++];
                for (int v : adj[u]) {
                    if (side[v] < 0) {
                        side[v] = !side[u];
                        bfs.push_back(v), S++;
                    } else if (side[u] == side[v]) {
                        return make_pair(false, move(side));
                    }
                }
            }
        }
    }

    return make_pair(true, move(side));
}

auto find_cycle(const vector<vector<int>>& adj) {
    int V = adj.size();

    vector<int8_t> color(V, 0);
    vector<int> parent(V, -1);
    array<int, 2> endp = {-1, -1};

    auto dfs = y_combinator([&](auto self, int u, int p) -> bool {
        parent[u] = p;
        color[u] = 1;
        for (int v : adj[u]) {
            if (v != p) {
                if (color[v] == 0 && self(v, u)) {
                    return true;
                } else if (color[v] == 1) {
                    endp = {u, v};
                    return true;
                }
            }
        }
        color[u] = 2;
        return false;
    });

    for (int u = 0; u < V; u++) {
        if (color[u] == 0 && dfs(u, -1)) {
            break;
        }
    }

    vector<int> nodes;

    if (endp[0] != -1) {
        int v = endp[1];
        nodes = {v};
        do {
            nodes.push_back(v = parent[v]);
        } while (v != endp[0]);
        reverse(begin(nodes), end(nodes));
    }
    return nodes;
}
