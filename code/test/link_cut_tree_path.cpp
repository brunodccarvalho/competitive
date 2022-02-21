#include "test_utils.hpp"
#include "struct/link_cut_tree_path.hpp"
#include "lib/slow_tree.hpp"
#include "lib/tree_action.hpp"

using lct_path = link_cut_tree_path<lct_node_path_sum<long>>;
using namespace tree_testing;

inline namespace detail {

bool stress_verify_link_cut(slow_tree<false>& slow, lct_path& tree, int D = 2) {
    assert(1 <= D && D <= 9);
    int N = slow.num_nodes();

    vector<vector<int>> path(D), path_length(D);
    vector<int> exp_path(N + 1), exp_path_length(N + 1);
    vector<int> nodes(N), above(N + 1);
    iota(begin(nodes), end(nodes), 1);

    bool ok_path = true, ok_path_length = true;

    for (int u = 1; u <= N; u++) {
        above[u] = slow.random_in_tree(u);
        exp_path[u] = slow.query_path(u, above[u]);
        exp_path_length[u] = slow.path_length(u, above[u]);
    }
    for (int d = 0; d < D; d++) {
        vector<int> order = nodes;
        shuffle(begin(order), end(order), mt);
        path[d].resize(N + 1);
        for (int u : order) {
            path[d][u] = tree.access_path(u, above[u])->path;
        }
        if (path[d] != exp_path) {
            if (ok_path) {
                ok_path = false;
                printcl("      above: {}\n", above);
                printcl("expect_path: {}\n", exp_path);
            }
            printcl("got_path[{}]: {}\n", d + 1, path[d]);
        }
    }
    for (int d = 0; d < D; d++) {
        vector<int> order = nodes;
        shuffle(begin(order), end(order), mt);
        path_length[d].resize(N + 1);
        for (int u : order) {
            path_length[d][u] = tree.access_path(u, above[u])->path_size;
        }
        if (path_length[d] != exp_path_length) {
            if (ok_path_length) {
                ok_path_length = false;
                printcl("             above: {}\n", above);
                printcl("expect_path_length: {}\n", exp_path_length);
            }
            printcl("got_path_length[{}]: {}\n", d + 1, path_length[d]);
        }
    }

    return ok_path && ok_path_length;
}

} // namespace detail

void stress_test_lct_path() {
    static actions_t<UnrootedAT> lct_stress_actions = {
        {UnrootedAT::LINK, 1500},        {UnrootedAT::CUT, 300},
        {UnrootedAT::LINK_CUT, 1200},    {UnrootedAT::FINDROOT, 1000},
        {UnrootedAT::LCA, 500},          {UnrootedAT::LCA_CONN, 1000},
        {UnrootedAT::QUERY_NODE, 2000},  {UnrootedAT::UPDATE_NODE, 1500},
        {UnrootedAT::UPDATE_PATH, 3500}, {UnrootedAT::QUERY_PATH, 3500},
        {UnrootedAT::PATH_LENGTH, 1500}, {UnrootedAT::STRESS_TEST, 400},
    };

    const int N = 200;
    slow_tree<false> slow(N);
    lct_path tree(N);
    auto actions = make_unrooted_actions(N, 400ms, lct_stress_actions, 9 * N / 10);
    bool ok = true;

    for (int ia = 0, A = actions.size(); ia < A; ia++) {
        print_regular(ia, A, 1000, "stress test lct path");
        auto [action, u, v, r, who, val] = actions[ia];
        string label;

        switch (action) {
        case UnrootedAT::LINK: {
            slow.link(u, v);
            bool fine = tree.link(u, v);
            assert(fine);
            label = format("[{}] LINK {}--{}", slow.num_trees(), u, v);
        } break;
        case UnrootedAT::CUT: {
            slow.cut(u, v);
            bool fine = tree.cut(u, v);
            assert(fine);
            label = format("[{}] CUT {}--{}", slow.num_trees(), u, v);
        } break;
        case UnrootedAT::FINDROOT: {
            tree.reroot(who);
            slow.reroot(who);
            int ans = tree.findroot(u);
            ok = ans == who;
            label = format("[{}] FINDROOT {}: ={} ?{}\n", slow.num_trees(), u, who, ans);
        } break;
        case UnrootedAT::LCA: {
            tree.reroot(r);
            slow.reroot(r);
            int ans = tree.lca(u, v);
            ok = ans == who;
            label = format("[{}] LCA {}..{}, {}: ={} ?{}", slow.num_trees(), u, v, r, who,
                           ans);
        } break;
        case UnrootedAT::QUERY_NODE: {
            long ans = tree.access_node(u)->self;
            ok = val == ans;
            label = format("[{}] QUERY {}: ={} ?{}", slow.num_trees(), u, val, ans);
        } break;
        case UnrootedAT::UPDATE_NODE: {
            long init = slow.query_node(u);
            slow.update_node(u, val);
            tree.access_node(u)->self = val;
            label = format("[{}] UPDATE {}: {}->{}", slow.num_trees(), u, init, val);
        } break;
        case UnrootedAT::QUERY_PATH: {
            long ans = tree.access_path(u, v)->path;
            ok = val == ans;
            label = format("[{}] QUERY PATH {}..{}: ={} ?{}", slow.num_trees(), u, v, val,
                           ans);
        } break;
        case UnrootedAT::PATH_LENGTH: {
            long ans = tree.access_path(u, v)->path_size;
            ok = val == ans;
            label = format("[{}] PATH LENGTH {}..{}: ={} ?{}", slow.num_trees(), u, v,
                           val, ans);
        } break;
        case UnrootedAT::UPDATE_PATH: {
            slow.update_path(u, v, val);
            tree.access_path(u, v)->add_path(val);
            label = format("[{}] UPDATE PATH {}..{}: {:+}", slow.num_trees(), u, v, val);
        } break;
        case UnrootedAT::STRESS_TEST: {
            ok = stress_verify_link_cut(slow, tree);
            label = format("[{}] STRESS TEST", slow.num_trees());
        } break;
        default:
            throw runtime_error("Unsupported action");
        }
        if (!ok) {
            printcl("Failed action: {}\n", action_names<UnrootedAT>.at(action));
        }
        assert(ok);
    }

    assert(ok);
}

void speed_test_lct_path() {
    static actions_t<UnrootedAT> lct_speed_topo_heavy_actions = {
        {UnrootedAT::LINK, 5000},        {UnrootedAT::CUT, 1000},
        {UnrootedAT::LINK_CUT, 4000},    {UnrootedAT::FINDROOT, 1000},
        {UnrootedAT::LCA, 800},          {UnrootedAT::LCA_CONN, 1200},
        {UnrootedAT::QUERY_NODE, 2000},  {UnrootedAT::UPDATE_NODE, 2500},
        {UnrootedAT::UPDATE_PATH, 2500}, {UnrootedAT::QUERY_PATH, 3500},
        {UnrootedAT::PATH_LENGTH, 1500},
    };
    static actions_t<UnrootedAT> lct_speed_update_heavy_actions = {
        {UnrootedAT::LINK, 1500},        {UnrootedAT::CUT, 300},
        {UnrootedAT::LINK_CUT, 1200},    {UnrootedAT::FINDROOT, 1000},
        {UnrootedAT::LCA, 800},          {UnrootedAT::LCA_CONN, 1200},
        {UnrootedAT::QUERY_NODE, 1000},  {UnrootedAT::UPDATE_NODE, 6000},
        {UnrootedAT::UPDATE_PATH, 8000}, {UnrootedAT::QUERY_PATH, 2400},
        {UnrootedAT::PATH_LENGTH, 600},
    };
    static actions_t<UnrootedAT> lct_speed_query_heavy_actions = {
        {UnrootedAT::LINK, 1500},        {UnrootedAT::CUT, 300},
        {UnrootedAT::LINK_CUT, 1200},    {UnrootedAT::FINDROOT, 1000},
        {UnrootedAT::LCA, 800},          {UnrootedAT::LCA_CONN, 1200},
        {UnrootedAT::QUERY_NODE, 5000},  {UnrootedAT::UPDATE_NODE, 1200},
        {UnrootedAT::UPDATE_PATH, 1800}, {UnrootedAT::QUERY_PATH, 10000},
        {UnrootedAT::PATH_LENGTH, 4000},
    };

    vector<vector<stringable>> table;
    table.push_back({"N", "name", "#actions", "time"});

    auto run = [&](int N, ms generation, const auto& name, const auto& freq) {
        lct_path tree(N);
        auto actions = make_unrooted_actions(N, generation, freq, N - 100);
        printcl("speed test lct path");

        START(linkcut);
        for (const auto& [action, u, v, r, who, val] : actions) {
            bool ok = true;
            switch (action) {
            case UnrootedAT::LINK: {
                tree.link(u, v);
            } break;
            case UnrootedAT::CUT: {
                tree.cut(u, v);
            } break;
            case UnrootedAT::FINDROOT: {
                tree.reroot(who);
                int ans = tree.findroot(u);
                ok = ans == who;
            } break;
            case UnrootedAT::LCA: {
                tree.reroot(r);
                int ans = tree.lca(u, v);
                ok = ans == who;
            } break;
            case UnrootedAT::QUERY_NODE: {
                long ans = tree.access_node(u)->self;
                ok = val == ans;
            } break;
            case UnrootedAT::UPDATE_NODE: {
                tree.access_node(u)->self = val;
            } break;
            case UnrootedAT::QUERY_PATH: {
                long ans = tree.access_path(u, v)->path;
                ok = val == ans;
            } break;
            case UnrootedAT::PATH_LENGTH: {
                long ans = tree.access_path(u, v)->path_size;
                ok = val == ans;
            } break;
            case UnrootedAT::UPDATE_PATH: {
                tree.access_path(u, v)->add_path(val);
            } break;
            default:
                throw runtime_error("Unsupported action");
            }
            if (!ok) {
                printcl("Failed action: {}\n", action_names<UnrootedAT>.at(action));
            }
            assert(ok);
        }
        TIME(linkcut);

        table.push_back({N, name, actions.size(), FORMAT_EACH(linkcut, actions.size())});
    };

    run(1000, 1s, "query heavy", lct_speed_query_heavy_actions);
    run(10000, 3s, "query heavy", lct_speed_query_heavy_actions);
    run(50000, 6s, "query heavy", lct_speed_query_heavy_actions);
    run(1000, 1s, "update heavy", lct_speed_update_heavy_actions);
    run(10000, 3s, "update heavy", lct_speed_update_heavy_actions);
    run(50000, 6s, "update heavy", lct_speed_update_heavy_actions);
    run(1000, 1s, "topo heavy", lct_speed_topo_heavy_actions);
    run(10000, 3s, "topo heavy", lct_speed_topo_heavy_actions);
    run(50000, 6s, "topo heavy", lct_speed_topo_heavy_actions);

    print_time_table(table, "LCT Path");
}

int main() {
    RUN_SHORT(stress_test_lct_path());
    RUN_SHORT(speed_test_lct_path());
    return 0;
}
