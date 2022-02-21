#pragma once

#include "struct/euler_tour_tree.hpp"
#include "struct/pbds.hpp"

struct online_dynamic_connectivity {
    template <typename K, typename V, typename Hash = std::hash<K>>
    using hash_table = gnu::cc_hash_table<K, V, Hash>;

    template <typename K, typename Hash = std::hash<K>>
    using hash_set = gnu::cc_hash_table<K, gnu::null_type, Hash>;

    struct dynacon_node {
        int id = 0;
        int subt_size = 0;  // how many nodes in subtree
        int adj[2] = {};    // how many adjacent to this node outside MST/inside MST
        int insubt[2] = {}; // how many adjacent to all nodes inside this subtree

        int tree_size() const { return subt_size; }
        int sum(int tree) const { return insubt[tree]; }

        void pushdown(bool, bool, dynacon_node&, dynacon_node&) {}

        void pushup(bool is_node, bool, dynacon_node& lhs, dynacon_node& rhs) {
            subt_size = is_node + lhs.subt_size + rhs.subt_size;
            for (int tree : {0, 1}) {
                if (adj[tree] > 0) {
                    insubt[tree] = id;
                } else {
                    insubt[tree] = max(lhs.insubt[tree], rhs.insubt[tree]);
                }
            }
        }
    };

    int N;
    vector<euler_tour_tree<dynacon_node>> ett;
    hash_table<pair<int, int>, int> edge_level;
    hash_table<pair<int, int>, hash_set<int>> adj[2];

    void ensure_level(int level) {
        if (int L = ett.size(); L == level) {
            ett.emplace_back(N);
            for (int u = 1; u <= N; u++) {
                ett[level].st[u].node.id = u;
            }
        }
    }

    explicit online_dynamic_connectivity(int N = 0) : N(N) { ensure_level(0); }

  public:
    // returns true if linking joined two unconnected components
    bool link(int u, int v) {
        if (u == v) {
            return false;
        } else if (ett[0].link(u, v)) {
            add_edge_level<1>(u, v, 0);
            return true;
        } else {
            add_edge_level<0>(u, v, 0);
            return false;
        }
    }

    bool cut(int u, int v) {
        int x, y;
        return cut(u, v, x, y);
    }

    // returns true if cutting separated a connected component into two
    // if another edge (fa,fb) was found to replace (u,v) it gets returned
    bool cut(int u, int v, int& fa, int& fb) {
        fa = fb = 0;
        if (u == v) {
            return false;
        }
        int level = get_edge_level(u, v);
        if (level == -1) {
            return false;
        }
        if (!ett[0].cut(u, v)) {
            rem_edge_level<0>(u, v, level);
            return false;
        }

        for (int i = level; i > 0; i--) {
            ett[i].cut(u, v);
        }
        rem_edge_level<1>(u, v, level);

        for (int i = level; i >= 0; i--) {
            int u_size = ett[i].access_node(u)->tree_size();
            int v_size = ett[i].access_node(v)->tree_size();
            if (u_size > v_size) {
                swap(u, v), swap(u_size, v_size);
            }

            // push tree edges down to level i+1
            while (true) {
                int a = find_with_bridge<1>(u, i);
                if (a == 0)
                    break;
                const auto& edges = adj[1].find({i, a})->second;
                int S = edges.size();
                while (S--) {
                    int b = *edges.begin();
                    rem_edge_level<1>(a, b, i);
                    add_edge_level<1>(a, b, i + 1);
                    ett[i + 1].link(a, b);
                }
            }

            while (true) {
                int a = find_with_bridge<0>(u, i);
                if (a == 0)
                    break;
                const auto& edges = adj[0].find({i, a})->second;
                int S = edges.size();
                while (S--) {
                    int b = *edges.begin();
                    if (ett[i].conn(b, v)) {
                        for (int j = 0; j <= i; j++) {
                            ett[j].link(a, b);
                        }
                        rem_edge_level<0>(a, b, i);
                        add_edge_level<1>(a, b, i);
                        fa = a, fb = b;
                        return false;
                    } else {
                        rem_edge_level<0>(a, b, i);
                        add_edge_level<0>(a, b, i + 1);
                    }
                }
            }
        }

        return true;
    }

    void reroot(int u) { ett[0].reroot(u); }

    int findroot(int u) { return ett[0].findroot(u); }

    bool conn(int u, int v) { return ett[0].conn(u, v); }

  private:
    int get_level(int u, int v) const {
        auto it = edge_level.find(minmax(u, v));
        return it == edge_level.end() ? -1 : it->second;
    }

    template <bool tree>
    int find_with_bridge(int u, int level) {
        if (int v = ett[level].access_node(u)->insubt[tree]; v != 0) {
            ett[level].access_node(v);
            return v;
        } else {
            return 0;
        }
    }

    template <bool tree>
    void add_edge_level(int u, int v, int level) {
        ensure_level(level);
        edge_level[minmax(u, v)] = level;
        adj[tree][{level, u}].insert(v);
        adj[tree][{level, v}].insert(u);
        ett[level].access_node(u)->adj[tree]++;
        ett[level].access_node(v)->adj[tree]++;
    }

    template <bool tree>
    void rem_edge_level(int u, int v, int level) {
        edge_level.erase(minmax(u, v));
        auto su = adj[tree].find({level, u}), sv = adj[tree].find({level, v});
        if (su->second.size() == 1u) {
            adj[tree].erase({level, u});
        } else {
            su->second.erase(v);
        }
        if (sv->second.size() == 1u) {
            adj[tree].erase({level, v});
        } else {
            sv->second.erase(u);
        }
        ett[level].access_node(u)->adj[tree]--;
        ett[level].access_node(v)->adj[tree]--;
    }

    int get_edge_level(int u, int v) const {
        auto it = edge_level.find(minmax(u, v));
        return it == edge_level.end() ? -1 : it->second;
    }
};
