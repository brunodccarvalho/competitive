#include "test_utils.hpp"
#include "lib/graph_generator.hpp"
#include "lib/graph_formats.hpp"

inline namespace detail {

static int si = 1;

#define SHOW_U(fn)                     \
    do {                               \
        auto g = fn;                   \
        showu(#fn, move(g), get_V(g)); \
    } while (0)

#define SHOW_D(fn)                     \
    do {                               \
        auto g = fn;                   \
        showd(#fn, move(g), get_V(g)); \
    } while (0)

#define SHOW_B(fn)                               \
    do {                                         \
        auto g = fn;                             \
        showb(#fn, move(g), get_U(g), get_W(g)); \
    } while (0)

#define SHOW_F(fn)                           \
    do {                                     \
        auto [g, s, t] = fn;                 \
        showf(#fn, move(g), get_V(g), s, t); \
    } while (0)

string pad(const string& lines) {
    stringstream ss(lines);
    string ans, line;
    while (getline(ss, line)) {
        ans += "  " + line + "\n";
    }
    return ans;
}

int get_V(const edges_t& g) {
    int V = 0;
    for (auto [u, v] : g)
        V = max(V, 1 + max(u, v));
    return V;
}
int get_U(const edges_t& g) {
    int U = 0;
    for (auto [u, v] : g)
        U = max(U, 1 + u);
    return U;
}
int get_W(const edges_t& g) {
    int V = 0;
    for (auto [u, v] : g)
        V = max(V, 1 + v);
    return V;
}

static string line_separator = string(70, '=');

void showu(string msg, edges_t&& g, int V) {
    sort(begin(g), end(g));
    print("{}\n", line_separator);
    print("{} # {}\n{}\n{}\n", //
          si++, msg, pad(to_human_undirected(g, V)), simple_dot(g, 0));
}
void showd(string msg, edges_t&& g, int V) {
    sort(begin(g), end(g));
    print("{}\n", line_separator);
    print("{} # {}\n{}\n{}\n", //
          si++, msg, pad(to_human_directed(g, V)), simple_dot(g, 1));
}
void showb(string msg, edges_t&& g, int U, int V) {
    sort(begin(g), end(g));
    print("{}\n", line_separator);
    print("{} # {}\n{}\n{}\n", //
          si++, msg, pad(to_human_bipartite(g, U, V)), simple_dot(g, 2));
}
void showf(string msg, edges_t&& g, int V, int s, int t) {
    sort(begin(g), end(g));
    print("{}\n", line_separator);
    print("{} # {} (s={} t={})\n{}\n{}\n", //
          si++, msg, s, t, pad(to_human_directed(g, V)), simple_dot(g, 1));
}

} // namespace detail

void print_graphs() {
    auto g = get<0>(random_geometric_flow_dag_connected(50, 0.04, 0.15));
    putln(simple_dot(g, true));

    g = random_geometric_undirected_connected(50, 0.04, 0.15);
    putln(simple_dot(g, false));

    g = random_geometric_tree(300, 0.15);
    putln(simple_dot(g, false));
}

void stress_test_geometric() {
    static vector<int> Vs = {50, 100, 200, 400, 600, 1000};
    static vector<double> ps = {.01, .05, .20, .60};
    static vector<double> as = {-.95,  -.75, -.40, -.10, -.01, -.001, 0,
                                +.001, +.01, +.10, +.40, +.75, +.95};
    const auto runtime = 30000ms / (Vs.size() * ps.size() * as.size());
    map<tuple<pair<int, double>, double, string>, stringable> times;

    auto run = [&](int V, double p, double alpha) {
        printcl("stress test geometric V,p,a={},{},{}", V, p, alpha);

        long Esum = 0;
        double expected = p * V * (V - 1) / 2;

        START(gen);
        LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
            auto [g, s, t] = random_geometric_flow_connected(V, p, p / 2, alpha);
            Esum += g.size();
        }
        TIME(gen);

        double actual = 1.0 * Esum / runs;

        times[{{V, alpha}, p, "time"}] = FORMAT_EACH(gen, runs);
        times[{{V, alpha}, p, "ratio"}] = format("{:.3f}", actual / expected);
        times[{{V, alpha}, p, "edges"}] = format("{:.3f}", actual);
        times[{{V, alpha}, p, "runs"}] = runs;
    };

    for (int V : Vs) {
        for (double p : ps) {
            for (double alpha : as) {
                run(V, p, alpha);
            }
        }
    }

    print_time_table(times, "Undirected geometric");
}

void visual_test_generators() {
    SHOW_D(random_binary_tree(20));
    SHOW_D(random_binary_tree(40));
    SHOW_D(random_binary_tree(80));

    SHOW_D(random_geometric_binary_tree(80, 0.1));
    SHOW_D(random_geometric_binary_tree(80, 0.2));
    SHOW_D(random_geometric_binary_tree(80, 0.35));
    SHOW_D(random_geometric_binary_tree(80, 0.5));

    SHOW_D(random_binary_tree(200));
    SHOW_D(random_binary_tree(200));
    SHOW_D(random_geometric_binary_tree(200, 0.2));
    SHOW_D(random_geometric_binary_tree(200, 0.2));
    SHOW_D(random_geometric_binary_tree(800, 0.12));
    SHOW_D(random_geometric_binary_tree(800, 0.08));

    SHOW_D(random_geometric_tree(40, -0.50));
    SHOW_D(random_geometric_tree(40, -0.25));
    SHOW_D(random_geometric_tree(40, -0.05));
    SHOW_D(random_geometric_tree(40, 0.05));
    SHOW_D(random_geometric_tree(40, 0.25));
    SHOW_D(random_geometric_tree(40, 0.50));

    SHOW_D(random_geometric_directed(30, 0.15, -0.50));
    SHOW_D(random_geometric_directed(30, 0.15, -0.25));
    SHOW_D(random_geometric_directed(30, 0.15, -0.12));
    SHOW_D(random_geometric_directed(30, 0.15, -0.05));
    SHOW_D(random_geometric_directed(30, 0.15, 0.05));
    SHOW_D(random_geometric_directed(30, 0.15, 0.12));
    SHOW_D(random_geometric_directed(30, 0.15, 0.25));
    SHOW_D(random_geometric_directed(30, 0.15, 0.50));

    SHOW_D(random_geometric_undirected(30, 0.15, -0.50));
    SHOW_D(random_geometric_undirected(30, 0.15, -0.25));
    SHOW_D(random_geometric_undirected(30, 0.15, -0.12));
    SHOW_D(random_geometric_undirected(30, 0.15, -0.05));
    SHOW_D(random_geometric_undirected(30, 0.15, 0.05));
    SHOW_D(random_geometric_undirected(30, 0.15, 0.12));
    SHOW_D(random_geometric_undirected(30, 0.15, 0.25));
    SHOW_D(random_geometric_undirected(30, 0.15, 0.50));

    SHOW_D(random_geometric_undirected_connected(30, 0.2, -0.50));
    SHOW_D(random_geometric_undirected_connected(30, 0.2, -0.25));
    SHOW_D(random_geometric_undirected_connected(30, 0.2, -0.05));
    SHOW_D(random_geometric_undirected_connected(30, 0.2, 0.05));
    SHOW_D(random_geometric_undirected_connected(30, 0.2, 0.25));
    SHOW_D(random_geometric_undirected_connected(30, 0.2, 0.50));

    SHOW_D(path_graph(10));
    SHOW_D(cycle_graph(10));

    SHOW_D(complete_graph(6));
    SHOW_D(complete2_graph(6));
    SHOW_D(complete_multipartite_graph({2, 3, 4, 5}));

    SHOW_D(random_tree(40));
    SHOW_D(random_forest(50, 4));
    SHOW_D(random_geometric_tree(40, -0.55));
    SHOW_D(random_geometric_tree(40, -0.30));
    SHOW_D(random_geometric_tree(40, 0.30));
    SHOW_D(random_geometric_tree(40, 0.55));

    SHOW_D(disjoint_complete_graph(5, 4));
    SHOW_D(disjoint_complete2_graph(5, 4));
    SHOW_D(one_connected_complete_graph(5, 4));
    SHOW_D(one_connected_complete2_graph(5, 4));
    SHOW_D(k_connected_complete_graph(6, 4, 2));
    SHOW_D(k_connected_complete2_graph(6, 4, 2));

    SHOW_U(random_regular(15, 6));
    SHOW_U(random_regular_connected(16, 4));
    SHOW_B(random_regular_bipartite(10, 25, 5));

    SHOW_U(random_uniform_undirected(25, 0.07));
    SHOW_D(random_uniform_directed(25, 0.035));
    SHOW_B(random_uniform_bipartite(10, 20, 0.07));
    SHOW_B(random_exact_bipartite(10, 20, 35));
    SHOW_U(random_exact_undirected(25, 40));
    SHOW_D(random_exact_directed(25, 40));

    SHOW_U(random_uniform_undirected_total(25, 0.05));
    SHOW_D(random_uniform_directed_total_in(25, 0.025));
    SHOW_D(random_uniform_directed_total_out(25, 0.025));
    SHOW_B(random_uniform_bipartite_total(10, 20, 0.05));

    SHOW_U(random_uniform_undirected_connected(25, 0.05));
    SHOW_D(random_uniform_directed_connected(25, 0.05));
    SHOW_U(random_exact_undirected_connected(25, 35));
    SHOW_D(random_exact_rooted_dag_connected(25, 35));
    SHOW_D(random_exact_directed_connected(25, 50));

    SHOW_F(random_uniform_flow_dag_connected(20, 0.03));
    SHOW_F(random_uniform_flow_connected(20, 0.03, 0.06));
    SHOW_F(random_geometric_flow_dag_connected(25, 0.12, +.3));
    SHOW_F(random_geometric_flow_dag_connected(25, 0.12, +.05));
    SHOW_F(random_geometric_flow_dag_connected(25, 0.12, -.05));
    SHOW_F(random_geometric_flow_dag_connected(25, 0.12, -.3));
    SHOW_F(random_geometric_flow_connected(25, 0.12, 0.06, +.3));
    SHOW_F(random_geometric_flow_connected(25, 0.12, 0.06, +.05));
    SHOW_F(random_geometric_flow_connected(25, 0.12, 0.06, -.05));
    SHOW_F(random_geometric_flow_connected(25, 0.12, 0.06, -.3));
}

int main() {
    print_graphs();
    RUN_BLOCK(visual_test_generators());
    RUN_BLOCK(stress_test_geometric());
    return 0;
}
