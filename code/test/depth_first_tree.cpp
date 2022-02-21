#include "test_utils.hpp"
#include "struct/depth_first_tree.hpp"
#include "lib/tree_action.hpp"

using dft_subtree = depth_first_tree<dft_node_sum<long>>;
using namespace tree_testing;

inline namespace detail {

bool stress_verify_dft(slow_tree<true>& slow, dft_subtree& tree, int D = 2) {
    assert(1 <= D && D <= 9);
    int N = slow.num_nodes();

    vector<vector<int>> subtree(D), subtree_size(D);
    vector<int> exp_subtree(N + 1), exp_subtree_size(N + 1);
    vector<int> nodes(N);
    iota(begin(nodes), end(nodes), 1);

    bool ok_subtree = true, ok_subtree_size = true;

    for (int u = 1; u <= N; u++) {
        exp_subtree[u] = slow.query_subtree(u);
        exp_subtree_size[u] = slow.subtree_size(u);
    }
    for (int d = 0; d < 0; d++) {
        vector<int> order = nodes;
        shuffle(begin(order), end(order), mt);
        subtree[d].resize(N + 1);
        for (int u : order) {
            subtree[d][u] = tree.access_subtree(u)->subtree();
            tree.end_access();
        }
        if (subtree[d] != exp_subtree) {
            if (ok_subtree) {
                ok_subtree = false;
                printcl("expect_subtree: {}", exp_subtree);
            }
            printcl("got_subtree[{}]: {}\n", d + 1, subtree[d]);
        }
    }
    for (int d = 0; d < D; d++) {
        vector<int> order = nodes;
        shuffle(begin(order), end(order), mt);
        subtree_size[d].resize(N + 1);
        for (int u : order) {
            subtree_size[d][u] = tree.access_subtree(u)->subtree_size();
            tree.end_access();
        }
        if (subtree_size[d] != exp_subtree_size) {
            if (ok_subtree_size) {
                ok_subtree_size = false;
                printcl("expect_subtree_size: {}\n", exp_subtree_size);
            }
            printcl("got_subtree_size[{}]: {}\n", d + 1, subtree_size[d]);
        }
    }

    return ok_subtree && ok_subtree_size;
}

} // namespace detail

void stress_test_dft() {
    static actions_t<RootedAT> dft_stress_actions = {
        {RootedAT::LINK, 2500},           {RootedAT::CUT, 500},
        {RootedAT::LINK_CUT, 2000},       {RootedAT::LCA, 0},
        {RootedAT::FINDROOT, 1500},       {RootedAT::LCA_CONN, 0},
        {RootedAT::QUERY_NODE, 2000},     {RootedAT::UPDATE_NODE, 3000},
        {RootedAT::UPDATE_SUBTREE, 3500}, {RootedAT::QUERY_SUBTREE, 3500},
        {RootedAT::SUBTREE_SIZE, 1500},   {RootedAT::STRESS_TEST, 400},
    };

    const int N = 200;
    slow_tree<true> slow(N);
    dft_subtree tree(N);
    auto actions = make_rooted_actions(N, 400ms, dft_stress_actions, 9 * N / 10);
    bool ok = true;

    for (int ia = 0, A = actions.size(); ia < A; ia++) {
        print_regular(ia, A, 1000, "stress test dft tree");
        auto [action, u, v, r, who, val] = actions[ia];
        string label;

        switch (action) {
        case RootedAT::LINK: {
            slow.link(u, v);
            tree.link(u, v);
            label = format("[{}] LINK {}--{}", slow.num_trees(), u, v);
        } break;
        case RootedAT::CUT: {
            slow.cut(u);
            tree.cut(u);
            label = format("[{}] CUT {}", slow.num_trees(), u);
        } break;
        case RootedAT::FINDROOT: {
            int ans = tree.findroot(u);
            ok = ans == who;
            label = format("[{}] FINDROOT {}: ={} ?{}\n", slow.num_trees(), u, who, ans);
        } break;
        // case RootedAT::LCA: {
        //     int ans = tree.lca(u, v);
        //     ok = who == ans;
        //     label = format("[{}] LCA {}..{}: ={} ?{}", slow.num_trees(), u, v, who,
        //     ans);
        // } break;
        case RootedAT::QUERY_NODE: {
            long ans = tree.access_node(u)->self;
            ok = val == ans;
            label = format("[{}] QUERY {}: ={} ?{}", slow.num_trees(), u, val, ans);
        } break;
        case RootedAT::UPDATE_NODE: {
            long init = slow.query_node(u);
            slow.update_node(u, val);
            tree.access_node(u)->self = val;
            label = format("[{}] UPDATE {}: {}->{}", slow.num_trees(), u, init, val);
        } break;
        case RootedAT::UPDATE_SUBTREE: {
            slow.update_subtree(u, val);
            tree.access_subtree(u)->add_subtree(val);
            tree.end_access();
            label = format("[{}] UPDATE SUBTREE {}: {:+}", slow.num_trees(), u, val);
        } break;
        case RootedAT::QUERY_SUBTREE: {
            long ans = tree.access_subtree(u)->subtree();
            tree.end_access();
            ok = val == ans;
            label = format("[{}] QUERY SUBTREE {}: ={} ?{}", slow.num_trees(), u, val,
                           ans);
        } break;
        case RootedAT::SUBTREE_SIZE: {
            long ans = tree.access_subtree(u)->subtree_size();
            tree.end_access();
            ok = val == ans;
            label = format("[{}] SUBTREE SIZE {}: ={} ?{}", slow.num_trees(), u, val,
                           ans);
        } break;
        case RootedAT::STRESS_TEST: {
            ok = stress_verify_dft(slow, tree);
            label = format("[{}] STRESS TEST", slow.num_trees());
        } break;
        default:
            throw runtime_error("Unsupported action");
        }
        if (!ok) {
            printcl("Failed action: {}\n", action_names<RootedAT>.at(action));
        }
        assert(ok);
    }

    assert(ok);
}

void speed_test_dft() {
    static actions_t<RootedAT> dft_speed_topo_heavy_actions = {
        {RootedAT::LINK, 5000},           {RootedAT::CUT, 1000},
        {RootedAT::LINK_CUT, 4000},       {RootedAT::LCA, 0},
        {RootedAT::FINDROOT, 1500},       {RootedAT::LCA_CONN, 0},
        {RootedAT::QUERY_NODE, 2000},     {RootedAT::UPDATE_NODE, 2500},
        {RootedAT::UPDATE_SUBTREE, 2500}, {RootedAT::QUERY_SUBTREE, 3500},
        {RootedAT::SUBTREE_SIZE, 1500},
    };
    static actions_t<RootedAT> dft_speed_update_heavy_actions = {
        {RootedAT::LINK, 1500},           {RootedAT::CUT, 500},
        {RootedAT::LINK_CUT, 1000},       {RootedAT::LCA, 0},
        {RootedAT::FINDROOT, 1500},       {RootedAT::LCA_CONN, 0},
        {RootedAT::QUERY_NODE, 1000},     {RootedAT::UPDATE_NODE, 6000},
        {RootedAT::UPDATE_SUBTREE, 8000}, {RootedAT::QUERY_SUBTREE, 2400},
        {RootedAT::SUBTREE_SIZE, 600},
    };
    static actions_t<RootedAT> dft_speed_query_heavy_actions = {
        {RootedAT::LINK, 1500},           {RootedAT::CUT, 500},
        {RootedAT::LINK_CUT, 1000},       {RootedAT::LCA, 0},
        {RootedAT::FINDROOT, 1500},       {RootedAT::LCA_CONN, 0},
        {RootedAT::QUERY_NODE, 5000},     {RootedAT::UPDATE_NODE, 1200},
        {RootedAT::UPDATE_SUBTREE, 1800}, {RootedAT::QUERY_SUBTREE, 10000},
        {RootedAT::SUBTREE_SIZE, 4000},
    };

    vector<vector<stringable>> table;
    table.push_back({"N", "name", "#actions", "time"});

    auto run = [&](int N, ms generation, const auto& name, const auto& freq) {
        dft_subtree tree(N);
        auto actions = make_rooted_actions(N, generation, freq, N - 100);
        printcl("speed test dft");

        START(dft);
        for (const auto& [action, u, v, r, who, val] : actions) {
            bool ok = true;
            switch (action) {
            case RootedAT::LINK: {
                tree.link(u, v);
            } break;
            case RootedAT::CUT: {
                tree.cut(u);
            } break;
            case RootedAT::FINDROOT: {
                int ans = tree.findroot(u);
                ok = who == ans;
            } break;
            // case RootedAT::LCA: {
            //     int ans = tree.lca(u, v);
            //     ok = who == ans;
            // } break;
            case RootedAT::QUERY_NODE: {
                long ans = tree.access_node(u)->self;
                ok = val == ans;
            } break;
            case RootedAT::UPDATE_NODE: {
                tree.access_node(u)->self = val;
            } break;
            case RootedAT::UPDATE_SUBTREE: {
                tree.access_subtree(u)->add_subtree(val);
                tree.end_access();
            } break;
            case RootedAT::QUERY_SUBTREE: {
                long ans = tree.access_subtree(u)->subtree();
                tree.end_access();
                ok = val == ans;
            } break;
            case RootedAT::SUBTREE_SIZE: {
                long ans = tree.access_subtree(u)->subtree_size();
                tree.end_access();
                ok = val == ans;
            } break;
            default:
                throw runtime_error("Unsupported action");
            }
            if (!ok) {
                printcl("Failed action: {}\n", action_names<RootedAT>.at(action));
            }
            assert(ok);
        }
        TIME(dft);

        table.push_back({N, name, actions.size(), FORMAT_EACH(dft, actions.size())});
    };

    run(1000, 1s, "query heavy", dft_speed_query_heavy_actions);
    run(10000, 3s, "query heavy", dft_speed_query_heavy_actions);
    run(50000, 6s, "query heavy", dft_speed_query_heavy_actions);
    run(1000, 1s, "update heavy", dft_speed_update_heavy_actions);
    run(10000, 3s, "update heavy", dft_speed_update_heavy_actions);
    run(50000, 6s, "update heavy", dft_speed_update_heavy_actions);
    run(1000, 1s, "topo heavy", dft_speed_topo_heavy_actions);
    run(10000, 3s, "topo heavy", dft_speed_topo_heavy_actions);
    run(50000, 6s, "topo heavy", dft_speed_topo_heavy_actions);

    print_time_table(table, "Depth First Tree");
}

int main() {
    RUN_BLOCK(stress_test_dft());
    RUN_BLOCK(speed_test_dft());
    return 0;
}
