#pragma once

#include "struct/disjoint_set.hpp"
#include "linear/basis.hpp"

/**
 * References:
 * Matroid Partitioning (1973), D. Knuth
 * Efficient Algorithms for a Family of Matroid Intersection Problems (1982), Gabow et al
 * Forests, Frames and Games: Algorithms for Matroid Sums (1989), Gabow et al
 * Optimal Matroid Partitioning Problems (2017), Y. Kawase et al
 *
 * MIT Combinatorial Optimization (2011), Goemans
 *      http://math.mit.edu/~goemans/18433S11/matroid-intersect-notes.pdf
 * Matroid Intersection (2009), Goemans
 *      https://math.mit.edu/~goemans/18438F09/lec11.pdf
 *
 * Oracle design: https://github.com/Aeren1564/Algorithms
 *                https://codeforces.com/contest/1556/submission/127657995
 */

// Core oracle functions:
//   - ground_size()
//         get the size of the ground set of this matroid
//   - check(i)   Primary verification
//         return whether the element i can be added to this independent set while
//         maintaining independency. i does not belong to the set currently.
//   - insert_check(i)   Verified insertion
//         try to add the element i to this independent set; if possible, do so and return
//         true, otherwise return false and leave the set unchanged.
//         can always be implemented as `return check(i) ? insert(i), true : false;`
//   - insert(i)   Unverified insertion
//         add the element i to this independent set. It is guaranteed that i can be
//         added to the independent set, and no verification need take place.
//
// EXCHANGE.
//   The matroid additionally supports exchange queries. This is usually important for
//   the algorithms to run efficiently, but is not required.
//   - exchange(i,j):
//         return true iff the element i currently in the set can be replaced by element j
//         should also return true if j can simply be added without removing i

struct matroid_stub {
    static constexpr bool EXCHANGE = true;

    matroid_stub(/* ... */) {}

    // size of the ground set for this matroid. Should be >0
    int ground_size() const { return -1; }

    // return true iff we can insert i
    bool check(int /*i*/) { return true; }

    // return false if we can't insert i, otherwise insert i and return true
    bool insert_check(int i) { return check(i) ? insert(i), true : false; }

    // insert i unchecked
    void insert(int /*i*/) {}

    // can we remove i and insert j? If this isn't easy, set EXCHANGE to false
    bool exchange(int /*i*/, int /*j*/) { return true; }

    // reset the current independent set to 0 elements, but keep the ground set
    void clear() {}
};

/**
 * Independent set iff <=k elements
 * Complexity: O(1) / O(1) / O(1)
 */
struct uniform_matroid {
    static constexpr bool EXCHANGE = true;
    int n, max_size;
    int count = 0;

    uniform_matroid(int n, int max_size) : n(n), max_size(max_size) {}

    int ground_size() const { return n; }

    bool check(int) { return count < max_size; }

    bool insert_check(int i) { return check(i) ? insert(i), true : false; }

    bool insert(int) { return count < max_size ? ++count, true : false; }

    bool exchange(int, int) { return true; }

    void clear() { count = 0; }
};

/**
// Independent set iff all elements have different colors
 * Complexity: O(1) / O(1) / O(C)
 */
struct colorful_matroid {
    static constexpr bool EXCHANGE = true;
    using Color = int16_t;
    vector<Color> color;
    vector<bool> taken;

    colorful_matroid(int C, vector<Color> color) : color(move(color)), taken(C, false) {}

    int ground_size() const { return color.size(); }

    int num_colors() const { return taken.size(); }

    bool check(int i) { return !taken[color[i]]; }

    bool insert_check(int i) { return check(i) ? insert(i), true : false; }

    void insert(int i) { taken[color[i]] = true; }

    bool exchange(int i, int j) { return color[i] == color[j] || check(j); }

    void clear() { taken.assign(num_colors(), false); }
};

/**
 * Independent set iff color i used at most limit[i] times for all i
 * Complexity: O(1) / O(1) / O(C)
 */
struct partition_matroid {
    static constexpr bool EXCHANGE = true;
    using Color = int16_t;
    using Limit = int16_t;
    vector<Color> color;
    vector<Limit> limit, count;

    partition_matroid(vector<Color> color, vector<Limit> limit)
        : color(move(color)), limit(move(limit)), count(this->limit) {}

    int ground_size() const { return color.size(); }

    int num_colors() const { return limit.size(); }

    bool check(int i) { return count[color[i]] > 0; }

    bool insert_check(int i) { return check(i) ? insert(i), true : false; }

    void insert(int i) { count[color[i]]--; }

    bool exchange(int i, int j) { return color[i] == color[j] || check(j); }

    void clear() { copy(begin(limit), end(limit), begin(count)); }
};

/**
 * Independent set iff edges do not induce a cycle
 * Complexity: O(1) / O(1) + O(V) once / O(V). Bad constant factor, prefer incremental
 */
struct graphic_matroid {
    static constexpr bool EXCHANGE = true;
    vector<array<int, 2>> g;
    vector<basic_string<int>> set;
    vector<int> tin, tout;
    disjoint_set_rollback dsu;
    int start = 0;
    bool needs_refresh = false;

    graphic_matroid(int V, vector<array<int, 2>> g)
        : g(move(g)), set(V), tin(V), tout(V), dsu(V) {}

    graphic_matroid(int V, vector<array<int, 2>> g, const disjoint_set_rollback& initial)
        : g(move(g)), set(V), tin(V), tout(V), dsu(initial), start(dsu.time()) {}

    int ground_size() const { return g.size(); }

    int num_vertices() const { return dsu.N; }

    bool check(int i) { return !dsu.same(g[i][0], g[i][1]); }

    bool insert_check(int i) { return check(i) ? insert(i), true : false; }

    void insert(int i) {
        auto [u, v] = g[i];
        dsu.join(u, v), needs_refresh = true;
        set[u].push_back(v), set[v].push_back(u);
    }

    bool exchange(int i, int j) {
        auto [u, v] = g[i];
        auto [a, b] = g[j];
        if (!dsu.same(a, b)) {
            return true;
        }
        refresh();
        int w = tin[u] < tin[v] ? v : u;
        return ancestor(w, a) != ancestor(w, b);
    }

    void clear() {
        for (int u = 0; u < dsu.N; u++) {
            set[u].clear();
        }
        dsu.rollback(start);
        needs_refresh = false;
    }

  private:
    bool ancestor(int i, int j) { return tin[i] <= tin[j] && tout[j] <= tout[i]; }

    void refresh() {
        if (needs_refresh) {
            needs_refresh = false;
            fill(begin(tin), end(tin), -1);
            for (int u = 0, timer = 0; u < dsu.N; u++) {
                if (tin[u] == -1) {
                    dfs(u, timer);
                }
            }
        }
    }

    void dfs(int u, int& timer) {
        tin[u] = timer++;
        for (int v : set[u]) {
            if (tin[v] == -1) {
                dfs(v, timer);
            }
        }
        tout[u] = timer++;
    }
};

/**
 * Independent set iff edges do not induce a cycle
 * Complexity: O(1) / -- / O(V)
 */
struct incremental_graphic_matroid {
    static constexpr bool EXCHANGE = false;
    vector<array<int, 2>> g;
    disjoint_set_rollback dsu;
    int start = 0;

    incremental_graphic_matroid(int V, vector<array<int, 2>> g) : g(move(g)), dsu(V) {}

    incremental_graphic_matroid(int V, vector<array<int, 2>> g,
                                const disjoint_set_rollback& initial)
        : g(move(g)), dsu(initial), start(dsu.time()) {}

    int ground_size() const { return g.size(); }

    int num_vertices() const { return dsu.N; }

    bool check(int i) { return !dsu.same(g[i][0], g[i][1]); }

    bool insert_check(int i) { return dsu.join(g[i][0], g[i][1]); }

    void insert(int i) { dsu.join(g[i][0], g[i][1]); }

    void clear() { dsu.rollback(start); }
};

/**
 * Independent set iff it is possible to remove 0/1 edges from each connected
 * component to obtain a forest.
 * Complexity: O(1) / -- / O(V)
 */
struct incremental_bicircular_matroid {
    static constexpr bool EXCHANGE = false;
    vector<array<int, 2>> g;
    vector<bool> cycle;
    disjoint_set dsu;

    incremental_bicircular_matroid(int V, vector<array<int, 2>> g)
        : g(move(g)), cycle(V), dsu(V) {}

    int ground_size() const { return g.size(); }

    int num_vertices() const { return dsu.N; }

    bool check(int i) {
        auto [u, v] = g[i];
        u = dsu.find(u), v = dsu.find(v);
        return !cycle[u] || !cycle[v];
    }

    bool insert_check(int i) { return check(i) ? insert(i), true : false; }

    void insert(int i) {
        auto [u, v] = g[i];
        u = dsu.find(u), v = dsu.find(v);
        cycle[u] = cycle[u] || cycle[v];
        dsu.join(u, v), cycle[dsu.find(u)] = cycle[u];
    }

    void clear() { dsu.assign(dsu.N), cycle.assign(dsu.N, false); }
};

/**
 * Independent set iff vectors are linearly independent
 * Complexity: O(1) / -- / O(dim·rank). Sometimes a bit better.
 */
template <typename T>
struct incremental_vector_matroid {
    static constexpr bool EXCHANGE = false;
    vector<vector<T>> space;
    vector_space_basis<T> basis, initial;

    incremental_vector_matroid(vector<vector<T>> space) : space(move(space)) {}

    incremental_vector_matroid(vector<vector<T>> space,
                               const vector_space_basis<T>& initial)
        : space(move(space)), initial(initial), basis(initial) {}

    int ground_size() const { return space.size(); }

    int dimensions() const { return space[0].size(); }

    bool check(int i) { return basis.check(space[i]); }

    bool insert_check(int i) { return basis.insert_check(space[i]); }

    void insert(int i) { basis.insert_check(space[i]); }

    void clear() { basis = initial; }
};

/**
// Independent set iff all binary vectors are linearly independent in Z2
 * Complexity: O(1) / -- / O(rank). Sometimes a bit better.
 */
template <typename T = uint64_t>
struct incremental_binary_matroid {
    static constexpr bool EXCHANGE = false;
    vector<T> space;
    integer_xor_basis<T> initial, basis;

    incremental_binary_matroid(vector<T> space) : space(move(space)) {}

    incremental_binary_matroid(vector<T> space, const integer_xor_basis<T>& initial)
        : space(move(space)), initial(initial), basis(initial) {}

    int ground_size() const { return space.size(); }

    bool check(int i) { return basis.check(space[i]); }

    bool insert_check(int i) { return basis.insert_check(space[i]); }

    void insert(int i) { basis.insert_check(space[i]); }

    void clear() { basis = initial; }
};

/**
 * Add constraint: independent set has size <=k elements
 * Complexity: same
 */
template <typename M>
struct truncated_matroid {
    static constexpr bool EXCHANGE = M::EXCHANGE;
    M matroid;
    int max_size;
    int count = 0;

    truncated_matroid(int max_size, M matroid)
        : matroid(move(matroid)), max_size(max_size) {}

    int ground_size() const { return matroid.ground_size(); }

    bool check(int i) { return count < max_size && matroid.check(i); }

    bool insert_check(int i) {
        return count < max_size && matroid.insert_check(i) ? count++, true : false;
    }

    void insert(int i) { count++, matroid.insert(i); }

    bool exchange(int i, int j) { return matroid.exchange(i, j); }

    void clear() { count = 0, matroid.clear(); }
};

/**
 * Add constraint: independent set has elements from the given 'allowed' universe
 * Complexity: same
 */
template <typename M>
struct subset_matroid {
    static constexpr bool EXCHANGE = M::EXCHANGE;
    M matroid;
    vector<bool> allowed;

    subset_matroid(vector<bool> allowed, M matroid)
        : allowed(move(allowed)), matroid(move(matroid)) {}

    int ground_size() const { return matroid.ground_size(); }

    bool check(int i) { return allowed[i] && matroid.check(i); }

    bool insert_check(int i) { return allowed[i] && matroid.insert_check(i); }

    void insert(int i) { matroid.insert(i); }

    bool exchange(int i, int j) { return allowed[j] && matroid.exchange(i, j); }

    void clear() { matroid.clear(); }
};

/**
 * Matroid sum (M1,M2) = (G1 U G2, I1 x I2).
 * Complexity: same
 */
template <typename M1, typename M2 = M1>
struct matroid_sum {
    static constexpr bool EXCHANGE = M1::EXCHANGE && M2::EXCHANGE;
    M1 a;
    M2 b;

    matroid_sum(M1 a, M2 b) : a(move(a)), b(move(b)) {}

    int ground_size() const { return a.ground_size() + b.ground_size(); }

    bool check(int i) {
        int s = a.ground_size();
        return i < s ? a.check(i) : b.check(i - s);
    }

    bool insert_check(int i) {
        int s = a.ground_size();
        return i < s ? a.insert_check(i) : b.insert_check(i - s);
    }

    void insert(int i) {
        int s = a.ground_size();
        i < s ? a.insert(i) : b.insert(i - s);
    }

    bool exchange(int i, int j) {
        if (int s = a.ground_size(); i < s && j < s) {
            return a.exchange(i, j);
        } else if (i >= s && j < s) {
            return a.check(j);
        } else if (i < s && j >= s) {
            return b.check(j - s);
        } else {
            return b.exchange(i - s, j - s);
        }
    }

    void clear() { a.clear(), b.clear(); }
};

/**
 * For testing purposes. Hold a matroid oracle of any type
 */
template <typename... Ms>
struct matroid_variant : variant<Ms...> {
    static constexpr bool EXCHANGE = (Ms::EXCHANGE && ...);

    using variant<Ms...>::variant;

    int ground_size() const {
        return std::visit([](auto&& m) { return m.ground_size(); }, self());
    }

    bool check(int i) {
        return std::visit([i](auto&& m) { return m.check(i); }, self());
    }

    bool insert_check(int i) {
        return std::visit([i](auto&& m) { return m.insert_check(i); }, self());
    }

    void insert(int i) {
        std::visit([i](auto&& m) { m.insert(i); }, self());
    }

    bool exchange(int i, int j) {
        return std::visit([i, j](auto&& m) { return m.exchange(i, j); }, self());
    }

    void clear() {
        std::visit([](auto&& m) { m.clear(); }, self());
    }

  private:
    inline auto& self() { return static_cast<variant<Ms...>&>(*this); }
    inline const auto& self() const { return static_cast<const variant<Ms...>&>(*this); }
};

template <typename Matroid, typename Cost>
struct cost_cutoff_matroid {
    static constexpr inline bool EXCHANGE = Matroid::EXCHANGE;
    Matroid matroid;
    const vector<Cost>& cost;
    Cost cut;

    cost_cutoff_matroid(Matroid matroid, const vector<Cost>& cost, Cost cut)
        : matroid(move(matroid)), cost(cost), cut(cut) {}

    int ground_size() const { return matroid.ground_size(); }

    bool check(int i) { return cost[i] <= cut && matroid.check(i); }

    bool insert_check(int i) { return cost[i] <= cut && matroid.insert_check(i); }

    void insert(int i) { return matroid.insert(i); }

    bool exchange(int i, int j) { return cost[j] <= cut && matroid.exchange(i, j); }

    void clear() { matroid.clear(); }
};

/**
 * Greedily find a basis
 * Complexity: O(n Ins)
 */
template <typename M>
auto find_basis(M matroid) {
    vector<int> basis;
    for (int i = 0, n = matroid.ground_size(); i < n; i++) {
        if (matroid.insert_check(i)) {
            basis.push_back(i);
        }
    }
    return basis;
}

/**
 * Minimum cost basis, Kruskal
 * Complexity: O(n log n + n Ins)
 */
template <typename M, typename Cost>
auto find_minimum_basis(const vector<Cost>& cost, M matroid) {
    int n = matroid.ground_size();
    vector<int> ground_set(n);
    iota(begin(ground_set), end(ground_set), 0);
    sort(begin(ground_set), end(ground_set),
         [&](int u, int v) { return cost[u] < cost[v]; });

    vector<int> basis;
    for (int i : ground_set) {
        if (matroid.insert_check(i)) {
            basis.push_back(i);
        }
    }
    return basis;
}

/**
 * Maximum cost basis, Kruskal
 * Complexity: O(n log n + n Ins)
 */
template <typename M, typename Cost>
auto find_maximum_basis(const vector<Cost>& cost, M matroid) {
    int n = matroid.ground_size();
    vector<int> ground_set(n);
    iota(begin(ground_set), end(ground_set), 0);
    sort(begin(ground_set), end(ground_set),
         [&](int u, int v) { return cost[u] > cost[v]; });

    vector<int> basis;
    for (int i : ground_set) {
        if (matroid.insert_check(i)) {
            basis.push_back(i);
        }
    }
    return basis;
}
