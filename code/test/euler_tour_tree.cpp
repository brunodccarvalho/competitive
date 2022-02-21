#include "test_utils.hpp"
#include "struct/euler_tour_tree.hpp"
#include "lib/tree_action.hpp"

using ett_subtree = euler_tour_tree<ett_node_sum<long>>;
using namespace tree_testing;

inline namespace detail {

bool stress_verify_ett(slow_tree<false>& slow, ett_subtree& tree, int D = 2) {
    assert(1 <= D && D <= 9);
    int N = slow.num_nodes();

    vector<vector<int>> subtree(D), subtree_size(D);
    vector<int> exp_tree(N + 1), exp_tree_size(N + 1);
    vector<int> nodes(N);
    iota(begin(nodes), end(nodes), 1);

    bool ok_subtree = true, ok_subtree_size = true;

    for (int u = 1; u <= N; u++) {
        slow.reroot(u);
        exp_tree[u] = slow.query_tree(u);
        exp_tree_size[u] = slow.tree_size(u);
    }
    for (int d = 0; d < 0; d++) {
        vector<int> order = nodes;
        shuffle(begin(order), end(order), mt);
        subtree[d].resize(N + 1);
        for (int u : order) {
            subtree[d][u] = tree.access_tree(u)->subtree();
        }
        if (subtree[d] != exp_tree) {
            if (ok_subtree) {
                ok_subtree = false;
                printcl("expect_subtree: {}\n", exp_tree);
            }
            printcl("got_subtree[{}]: {}\n", d + 1, subtree[d]);
        }
    }
    for (int d = 0; d < D; d++) {
        vector<int> order = nodes;
        shuffle(begin(order), end(order), mt);
        subtree_size[d].resize(N + 1);
        for (int u : order) {
            subtree_size[d][u] = tree.access_tree(u)->subtree_size();
        }
        if (subtree_size[d] != exp_tree_size) {
            if (ok_subtree_size) {
                ok_subtree_size = false;
                printcl("expect_subtree_size: {}\n", exp_tree_size);
            }
            printcl("got_subtree_size[{}]: {}\n", d + 1, subtree_size[d]);
        }
    }

    return ok_subtree && ok_subtree_size;
}

} // namespace detail

void stress_test_ett() {
    static actions_t<UnrootedAT> ett_stress_actions = {
        {UnrootedAT::LINK, 2500},          {UnrootedAT::CUT, 500},
        {UnrootedAT::LINK_CUT, 2000},      {UnrootedAT::LCA, 0},
        {UnrootedAT::FINDROOT, 1500},      {UnrootedAT::LCA_CONN, 0},
        {UnrootedAT::QUERY_NODE, 2000},    {UnrootedAT::UPDATE_NODE, 3000},
        {UnrootedAT::UPDATE_TREE, 3500},   {UnrootedAT::QUERY_TREE, 3500},
        {UnrootedAT::QUERY_SUBTREE, 2000}, {UnrootedAT::UPDATE_SUBTREE, 2000},
        {UnrootedAT::SUBTREE_SIZE, 2000},  {UnrootedAT::TREE_SIZE, 1500},
        {UnrootedAT::STRESS_TEST, 400},
    };

    const int N = 200;
    slow_tree<false> slow(N);
    ett_subtree tree(N);
    auto actions = make_unrooted_actions(N, 1s, ett_stress_actions, 9 * N / 10, true);
    bool ok = true;

    for (int ia = 0, A = actions.size(); ia < A; ia++) {
        print_regular(ia, A, 10, "stress test euler tour tree");
        auto [action, u, v, r, who, val] = actions[ia];
        string label;
        long ans = -1;

        switch (action) {
        case UnrootedAT::LINK: {
            slow.link(u, v);
            ok = tree.link(u, v);
            label = format("[{}] LINK {}--{}", slow.num_trees(), u, v);
        } break;
        case UnrootedAT::CUT: {
            slow.cut(u, v);
            ok = tree.cut(u, v);
            label = format("[{}] CUT {}--{}", slow.num_trees(), u, v);
        } break;
        case UnrootedAT::FINDROOT: {
            tree.reroot(who);
            slow.reroot(who);
            ans = tree.findroot(u);
            ok = ans == who;
            label = format("[{}] FINDROOT {}: ={} ?{}", slow.num_trees(), u, who, ans);
        } break;
        // case UnrootedAT::LCA: {
        //     tree.reroot(r);
        //     slow.reroot(r);
        //     int ans = tree.lca(u, v);
        //     ok = ans == who;
        //     label = format("[{}] LCA {}..{}, {}: ={} ?{}", slow.num_trees(), u, v, r,
        //     who, ans);
        // } break;
        case UnrootedAT::QUERY_NODE: {
            ans = tree.access_node(u)->self;
            ok = val == ans;
            label = format("[{}] QUERY {}: ={} ?{}", slow.num_trees(), u, val, ans);
        } break;
        case UnrootedAT::UPDATE_NODE: {
            long init = slow.query_node(u);
            slow.update_node(u, val);
            tree.access_node(u)->self = val;
            label = format("[{}] UPDATE {}: {}->{}", slow.num_trees(), u, init, val);
        } break;
        case UnrootedAT::QUERY_TREE: {
            ans = tree.access_tree(u)->subtree();
            ok = val == ans;
            label = format("[{}] QUERY TREE {}: ={} ?{}", slow.num_trees(), u, val, ans);
        } break;
        case UnrootedAT::TREE_SIZE: {
            ans = tree.access_tree(u)->subtree_size();
            ok = val == ans;
            label = format("[{}] TREE SIZE {}: ={} ?{}", slow.num_trees(), u, val, ans);
        } break;
        case UnrootedAT::UPDATE_TREE: {
            slow.reroot(u);
            slow.update_tree(u, val);
            tree.access_tree(u)->lazy += val;
            label = format("[{}] UPDATE TREE {}: {:+}", slow.num_trees(), u, val);
        } break;
        case UnrootedAT::QUERY_SUBTREE: {
            ans = tree.access_subtree(u, v)->subtree();
            tree.end_access();
            ok = val == ans;
            label = format("[{}] QUERY TREE {}: ={} ?{}", slow.num_trees(), u, val, ans);
        } break;
        case UnrootedAT::SUBTREE_SIZE: {
            ans = tree.access_subtree(u, v)->subtree_size();
            tree.end_access();
            ok = val == ans;
            label = format("[{}] TREE SIZE {}: ={} ?{}", slow.num_trees(), u, val, ans);
        } break;
        case UnrootedAT::UPDATE_SUBTREE: {
            slow.update_subtree(u, v, val);
            tree.access_subtree(u, v)->add_subtree(val);
            tree.end_access();
            label = format("[{}] UPDATE TREE {}: {:+}", slow.num_trees(), u, val);
        } break;
        case UnrootedAT::STRESS_TEST: {
            ok = stress_verify_ett(slow, tree);
            label = format("[{}] STRESS TEST", slow.num_trees());
        } break;
        default:
            throw runtime_error("Unsupported action");
        }
        if (!ok) {
            printcl("Failed action: {}\n", action_names<UnrootedAT>.at(action));
            printcl("val, ans: {}, {}\n", val, ans);
        }
        assert(ok);
    }
}

void speed_test_ett() {
    static actions_t<UnrootedAT> ett_speed_topo_heavy_actions = {
        {UnrootedAT::LINK, 5000},           {UnrootedAT::CUT, 1000},
        {UnrootedAT::LINK_CUT, 4000},       {UnrootedAT::LCA, 0},
        {UnrootedAT::FINDROOT, 1000},       {UnrootedAT::LCA_CONN, 0},
        {UnrootedAT::QUERY_NODE, 2000},     {UnrootedAT::UPDATE_NODE, 2500},
        {UnrootedAT::UPDATE_TREE, 2500},    {UnrootedAT::QUERY_TREE, 3500},
        {UnrootedAT::TREE_SIZE, 1500},      {UnrootedAT::QUERY_SUBTREE, 2000},
        {UnrootedAT::UPDATE_SUBTREE, 2000}, {UnrootedAT::SUBTREE_SIZE, 2000},
    };
    static actions_t<UnrootedAT> ett_speed_update_heavy_actions = {
        {UnrootedAT::LINK, 1500},           {UnrootedAT::CUT, 300},
        {UnrootedAT::LINK_CUT, 1200},       {UnrootedAT::LCA, 0},
        {UnrootedAT::FINDROOT, 1000},       {UnrootedAT::LCA_CONN, 0},
        {UnrootedAT::QUERY_NODE, 1000},     {UnrootedAT::UPDATE_NODE, 6000},
        {UnrootedAT::UPDATE_TREE, 8000},    {UnrootedAT::QUERY_TREE, 2400},
        {UnrootedAT::TREE_SIZE, 600},       {UnrootedAT::QUERY_SUBTREE, 2000},
        {UnrootedAT::UPDATE_SUBTREE, 2000}, {UnrootedAT::SUBTREE_SIZE, 2000},
    };
    static actions_t<UnrootedAT> ett_speed_query_heavy_actions = {
        {UnrootedAT::LINK, 1500},           {UnrootedAT::CUT, 300},
        {UnrootedAT::LINK_CUT, 1200},       {UnrootedAT::LCA, 0},
        {UnrootedAT::FINDROOT, 1000},       {UnrootedAT::LCA_CONN, 0},
        {UnrootedAT::QUERY_NODE, 5000},     {UnrootedAT::UPDATE_NODE, 1200},
        {UnrootedAT::UPDATE_TREE, 2000},    {UnrootedAT::QUERY_TREE, 10000},
        {UnrootedAT::TREE_SIZE, 4000},      {UnrootedAT::QUERY_SUBTREE, 2000},
        {UnrootedAT::UPDATE_SUBTREE, 2000}, {UnrootedAT::SUBTREE_SIZE, 2000},
    };

    vector<vector<stringable>> table;
    table.push_back({"N", "name", "#actions", "time"});

    auto run = [&](int N, ms generation, const auto& name, const auto& freq) {
        ett_subtree tree(N);
        auto actions = make_unrooted_actions(N, generation, freq, N - 100, true);
        printcl("speed test ett");

        START(ett);
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
            // case UnrootedAT::LCA: {
            //     tree.reroot(r);
            //     int ans = tree.lca(u, v);
            //     ok = ans == who;
            // } break;
            case UnrootedAT::QUERY_NODE: {
                long ans = tree.access_node(u)->self;
                ok = val == ans;
            } break;
            case UnrootedAT::UPDATE_NODE: {
                tree.access_node(u)->self = val;
            } break;
            case UnrootedAT::QUERY_TREE: {
                long ans = tree.access_tree(u)->subtree();
                ok = val == ans;
            } break;
            case UnrootedAT::TREE_SIZE: {
                long ans = tree.access_tree(u)->subtree_size();
                ok = val == ans;
            } break;
            case UnrootedAT::UPDATE_TREE: {
                tree.access_tree(u)->lazy += val;
            } break;
            case UnrootedAT::QUERY_SUBTREE: {
                long ans = tree.access_subtree(u, v)->subtree();
                tree.end_access();
                ok = val == ans;
            } break;
            case UnrootedAT::SUBTREE_SIZE: {
                long ans = tree.access_subtree(u, v)->subtree_size();
                tree.end_access();
                ok = val == ans;
            } break;
            case UnrootedAT::UPDATE_SUBTREE: {
                tree.access_subtree(u, v)->add_subtree(val);
                tree.end_access();
            } break;
            default:
                throw runtime_error("Unsupported action");
            }
            if (!ok) {
                printcl("Failed action: {}\n", action_names<UnrootedAT>.at(action));
            }
            assert(ok);
        }
        TIME(ett);

        table.push_back({N, name, actions.size(), FORMAT_EACH(ett, actions.size())});
    };

    run(1000, 1s, "query heavy", ett_speed_query_heavy_actions);
    run(10000, 3s, "query heavy", ett_speed_query_heavy_actions);
    run(50000, 6s, "query heavy", ett_speed_query_heavy_actions);
    run(1000, 1s, "update heavy", ett_speed_update_heavy_actions);
    run(10000, 3s, "update heavy", ett_speed_update_heavy_actions);
    run(50000, 6s, "update heavy", ett_speed_update_heavy_actions);
    run(1000, 1s, "topo heavy", ett_speed_topo_heavy_actions);
    run(10000, 3s, "topo heavy", ett_speed_topo_heavy_actions);
    run(50000, 6s, "topo heavy", ett_speed_topo_heavy_actions);

    print_time_table(table, "Euler Tour Tree");
}

int main() {
    RUN_BLOCK(stress_test_ett());
    RUN_BLOCK(speed_test_ett());
    return 0;
}
