#pragma once

#include "parallel/priority_thread_pool.hpp" // priority_thread_pool
#include "parallel/spinlock.hpp"             // concurrent_queue

/**
 * Check fn_orchestrator.hpp for an explanation of what an orchestrator does
 *
 * The second orchestrator version uses an explicit graph. The amount of work done in
 * the the main thread is O(E) locks/unlocks, where E is the number of edges in the
 * graph. Cycles are allowed to exist: nodes in cycles are simply ignored.
 */
struct graph_orchestrator {
  private:
    int N, E;
    vector<int> adj, off, deps;

    auto toposort() const {
        vector<int> cnt(N, 0), dfs;
        int j = 0, S = 0;

        for (int u = 0; u < N; u++)
            if (deps[u] == 0)
                dfs.push_back(u), S++;

        while (j < S) {
            int u = dfs[j++];
            for (int i = off[u]; i < off[u + 1]; i++)
                if (int v = adj[i]; ++cnt[v] == deps[v])
                    dfs.push_back(v), S++;
        }

        return dfs;
    }

  public:
    explicit graph_orchestrator(int N, const vector<array<int, 2>>& g)
        : N(N), E(g.size()), adj(E), off(N + 1, 0), deps(N, 0) {
        for (auto [u, v] : g)
            off[u + 1]++, deps[v]++;
        partial_sum(begin(off), end(off), begin(off));
        auto cur = off;
        for (auto [u, v] : g)
            adj[cur[u]++] = v;
    }

    bool verify() const { return int(toposort().size()) == N; }

    template <typename Fn>
    void sequential_make(const Fn& job) {
        for (int u : toposort())
            job(u);
    }

    template <typename Fn>
    void concurrent_make(const Fn& job, int nthreads) {
        vector<int> cnt(N, 0);
        priority_thread_pool<int> pool(nthreads);
        concurrent_queue<int> done(N);
        int seen = 0;

        auto runner = [&job, &done](int u) { job(u), done.push(u); };

        for (int u = 0; u < N; u++)
            if (deps[u] == 0)
                pool.submit(off[u + 1] - off[u], runner, u), seen++;

        while (seen < N) {
            pool.wait_for(1);
            while (!done.empty()) {
                int u = done.pop();
                for (int i = off[u]; i < off[u + 1]; i++) {
                    int v = adj[i];
                    if (++cnt[v] == deps[v])
                        pool.submit(off[v + 1] - off[v], runner, v), seen++;
                }
            }
        }

        pool.finish();
    }
};
