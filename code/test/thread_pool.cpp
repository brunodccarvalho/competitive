#include "test_utils.hpp"
#include "parallel/thread_pool.hpp"
#include "lib/graph_generator.hpp"
#include "linear/matrix.hpp"
#include "parallel/fn_orchestrator.hpp"
#include "parallel/graph_orchestrator.hpp"
#include "parallel/priority_thread_pool.hpp"

using mat_t = mat<unsigned>;

template <typename RNG>
mat_t makemat(int n, RNG& rnd) {
    uniform_int_distribution<unsigned> distv(0, 1'000'000);
    mat_t mat({n, n});
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            mat[i][j] = distv(rnd);
    return mat;
}

// Choose a random n and generate two nxn matrices and multiply them, discard the result
void compute(int id) {
    intd distn(200, 500);
    default_random_engine rnd(random_device{}());
    int n = distn(rnd);
    auto a = makemat(n, rnd);
    auto b = makemat(n, rnd);
    auto c = a * b;
    string s = format(" {:4} product complete, n: {:3}", id, n);
    puts(s.data());
}

// Choose a random n and generate two nxn matrices and multiply them, discard the result
void compute_priority(int id, int priority) {
    intd distn(200, 500);
    default_random_engine rnd(random_device{}());
    int n = distn(rnd);
    auto a = makemat(n, rnd);
    auto b = makemat(n, rnd);
    auto c = a * b;
    string s = format(" {:4} product complete, n: {:3}, p: {:4}", id, n, priority);
    puts(s.data());
}

template <typename T>
struct action_t {
    int b1, b2;
    T value;
};

void stress_test_pool_submit(int N = 200, int rate = 47, int nthreads = 5) {
    thread_pool pool(nthreads);
    assert(pool.pool_size() == nthreads);
    int i = 1;
    while (i <= N) {
        int j = 0;
        while (i <= N && j < rate) {
            pool.submit(compute, i), i++, j++;
        }
        string s = format("-- {:4}..{:4}, added {:4} jobs", i - j, i - 1, j);
        puts(s.data());
        if (j == rate) {
            pool.wait();
        } else {
            pool.cancel();
        }
    }
}

void stress_test_priority_pool_submit(int N = 200, int rate = 47, int nthreads = 5) {
    priority_thread_pool<int> pool(nthreads);
    assert(int(pool.pool_size()) == nthreads);
    intd priorityd(1000, 9999);
    int i = 1;
    while (i <= N) {
        int j = 0;
        while (i <= N && j < rate) {
            int p = priorityd(mt);
            pool.submit(p, compute_priority, i, p), i++, j++;
        }
        string s = format("-- {:4}..{:4}, added {:4} jobs", i - j, i - 1, j);
        puts(s.data());
        if (j == rate) {
            pool.wait();
        } else {
            pool.cancel();
        }
    }
}

void speed_test_fn_orchestrator() {
    static vector<int> Vs = {100, 500, 1000, 2000};
    static vector<int> Bs = {3, 10, 20, 50};
    map<tuple<int, int, string>, string> table;

    auto run = [&](int V, int buckets, int n, int nthreads) {
        vector<mat_t> vis(buckets, mat_t::identity(n));
        vector<mat_t> vis2(buckets, mat_t::identity(n));
        vector<action_t<mat_t>> val(V);

        intd buckd(0, buckets - 1);
        intd numd(1, 100'000'000);
        for (int i = 0; i < V; i++) {
            val[i] = {buckd(mt), buckd(mt), makemat(n, mt)};
        }

        auto job = [&](int u) {
            auto [b1, b2, mat] = val[u];
            vis[b1] = vis[b1] * mat;
            vis[b2] = vis[b2] * mat;
        };

        auto depends = [&val](int u, int v) {
            int b1 = val[u].b1, b2 = val[u].b2, b3 = val[v].b1, b4 = val[v].b2;
            return u < v && (b1 == b3 || b1 == b4 || b2 == b3 || b2 == b4);
        };

        fn_orchestrator orch(V, depends);

        START(sequential);
        orch.sequential_make(job);
        TIME(sequential);

        swap(vis, vis2);

        START(concurrent);
        orch.concurrent_make(job, nthreads);
        TIME(concurrent);

        assert(vis == vis2);

        table[{V, buckets, "seq"}] = FORMAT_TIME(sequential);
        table[{V, buckets, "conc"}] = FORMAT_TIME(concurrent);
    };

    for (int V : Vs) {
        for (int B : Bs) {
            run(V, B, 60, 8);
        }
    }

    print_time_table(table, "Function orchestrator");
}

void speed_test_graph_orchestrator() {
    vector<int> Vs = {200, 500, 1000, 2000, 4000};
    vector<int> Es = {2, 3, 5, 10, 20};
    map<tuple<int, int, string>, string> table;

    auto run = [&](int V, int E, int n, int nthreads) {
        printcl("speed test graph orchestrator V,E,n={},{},{}", V, V * E, n);

        vector<mat_t> vis(V), vis2(V);
        for (int i = 0; i < V; i++)
            vis[i] = vis2[i] = makemat(n, mt);

        auto g = random_exact_rooted_dag_connected(V, V * E);
        auto rev = make_adjacency_lists_reverse(V, g);

        auto job = [&](int u) {
            for (int v : rev[u])
                vis[u] = vis[v] * vis[u];
        };

        graph_orchestrator orch(V, g);

        START(sequential);
        orch.sequential_make(job);
        TIME(sequential);

        swap(vis, vis2);

        START(concurrent);
        orch.concurrent_make(job, nthreads);
        TIME(concurrent);

        assert(vis == vis2);

        table[{V, E, "seq"}] = FORMAT_TIME(sequential);
        table[{V, E, "conc"}] = FORMAT_TIME(concurrent);
    };

    for (int V : Vs) {
        for (int E : Es) {
            run(V, E, 60, 8);
        }
    }

    print_time_table(table, "Graph orchestrator");
}

int main() {
    setbuf(stdout, nullptr), setbuf(stderr, nullptr);
    RUN_BLOCK(speed_test_graph_orchestrator());
    RUN_BLOCK(speed_test_fn_orchestrator());
    RUN_BLOCK(stress_test_pool_submit());
    RUN_BLOCK(stress_test_priority_pool_submit());
    return 0;
}
