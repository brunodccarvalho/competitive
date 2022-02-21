#pragma once

#include "struct/integer_lists.hpp"          // linked_lists
#include "parallel/priority_thread_pool.hpp" // priority_thread_pool
#include "parallel/spinlock.hpp"             // concurrent_queue

/**
 * An orchestrator is like classic unix make.
 *
 * There is a (possibly implicit) graph made up of N nodes.
 * For each node u, we want to run some work, job(u).
 *
 * Between the nodes are dependency relations: for nodes u,v, it may be required that
 * job(u) be processed before job(v). In a directed graph this is analogous to an edge
 * from u to v. To satisfy the dependency relation requirements, the jobs must be
 * processed in topological order.
 *
 * However, we would like to run the jobs in parallel if possible, so while running
 * the jobs one by one in topological order is sufficient to guarantee correctness, it
 * is usually excessive as there won't be so many interdependencies: in most use
 * cases, neither u will depend on v nor v depend on u, so they can be processed in
 * parallel if all their dependencies have been processed.
 *
 * This is the practical solution to the job-scheduling problem, where we don't wan't
 * to optimize anything (like makespan or cost), we just want to run the damn jobs.
 *
 * For simplicity the orchestrator operates over the integers [0...N).
 */

/**
 * The first orchestrator version uses an implicit graph and is governed by a
 * black-box dependency function D. The function D is such that D(u,v) is true if and
 * only if v depends on u (that is, u must be processed before v). We assume that the
 * ordering of nodes 0...N-1 is a valid topological order of the nodes. The function D
 * does not need to be a valid comparison function; in particular, it does not need to
 * be transitive nor total.
 *
 * Since D is a black-box the main thread needs to use heuristics to govern the amount
 * of time it spends looking for new runnable jobs and the amount of time it spends
 * waiting for the current jobs to finish running, and therefore allow earlier jobs to
 * be started. I don't see an easy solution to this - searching for too long to
 * collect dependencies might mean we take too long processing a finished job;
 * searching too little might mean we miss runnable jobs.
 *
 * In this implementation, the main thread is greedy and will search for runnable jobs
 * until the memory requirements of the dependency tracking exceed 2N + T^2 integers,
 * where T is the number of threads. Also, the concurrent queue makes it so that no real
 * synchronization is needed between the runners and the main thread.
 * If the jobs don't run too quickly, this requires O(N + T^2) memory and at any point
 * guarantees at least O(N^1/2) of the pending jobs have their dependencies completely
 * determined. Also, the nodes are processed by priority, which is their number of
 * dependents at the time of insertion.
 */
template <typename Depends>
struct fn_orchestrator {
  private:
    int N;
    Depends d;

  public:
    explicit fn_orchestrator(int N, const Depends& d = Depends()) : N(N), d(d) {}

    bool verify() const {
        for (int u = 1; u < N; u++)
            if (d(u, u - 1))
                return false;
        return true;
    }

    template <typename Fn>
    void sequential_make(const Fn& job) {
        for (int u = 0; u < N; u++)
            job(u);
    }

    template <typename Fn>
    void concurrent_make(const Fn& job, int nthreads) {
        vector<int> cnt(N, -1);
        vector<vector<int>> dependents(N);
        linked_lists open(1, N); // 1 list only
        priority_thread_pool<int> pool(nthreads);
        concurrent_queue<int> done(N);
        unsigned long heavy = 0;
        const unsigned long maxmem = 1L * nthreads * nthreads + 2L * N;

        auto runner = [&job, &done](int u) { job(u), done.push(u); };

        for (int u = 0; u < N; u++) {
            open.push_back(0, u);
        }

        for (int v = 0; v < N; v++) {
            if (heavy >= maxmem) {
                auto needed = max(1ul, (heavy - maxmem) / N);
                pool.wait_for(needed);
            }
            while (!done.empty()) {
                int u = done.pop();
                for (int w : dependents[u]) {
                    if (--cnt[w] == 0) {
                        int priority = dependents[w].size();
                        pool.submit(priority, runner, w);
                    }
                }
                heavy -= dependents[u].size();
                vector<int> empty;
                swap(dependents[u], empty);
                open.erase(u);
            }
            cnt[v] = 0;
            FOR_EACH_IN_LINKED_LIST (u, 0, open) {
                if (u == v)
                    break;
                if (d(u, v)) {
                    dependents[u].push_back(v);
                    cnt[v]++;
                }
            }
            heavy += cnt[v];
            if (cnt[v] == 0) {
                pool.submit(-1, runner, v);
            }
        }

        while (heavy > 0) {
            pool.wait_for(1);
            while (!done.empty()) {
                int u = done.pop();
                for (int w : dependents[u]) {
                    if (--cnt[w] == 0) {
                        int priority = dependents[w].size();
                        pool.submit(priority, runner, w);
                    }
                }
                heavy -= dependents[u].size();
                vector<int> empty;
                swap(dependents[u], empty);
                open.erase(u);
            }
        }

        pool.finish();
    }
};
