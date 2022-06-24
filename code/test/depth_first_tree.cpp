#include "test_utils.hpp"
#include "struct/depth_first_tree.hpp"
#include "lib/tree_action.hpp"

using namespace tree_testing;

void stress_test_depth_first_tree() {
    static actions_t<RootedAT> stress_actions = {
        {RootedAT::LINK, 1500},     //
        {RootedAT::CUT, 300},       //
        {RootedAT::LINK_CUT, 1200}, //
        {RootedAT::FINDROOT, 1000}, //
        // {RootedAT::LCA, 500},       //
        // {RootedAT::LCA_CONN, 1000}, //

        {RootedAT::UPDATE_NODE, 1500}, //
        {RootedAT::QUERY_NODE, 2000},  //

        // {RootedAT::UPDATE_PATH, 3500}, //
        // {RootedAT::QUERY_PATH, 3500},  //
        // {RootedAT::PATH_LENGTH, 1500}, //

        {RootedAT::UPDATE_SUBTREE, 3500}, //
        {RootedAT::QUERY_SUBTREE, 3500},  //
        {RootedAT::SUBTREE_SIZE, 1500},   //

        // {RootedAT::UPDATE_TREE, 2500}, //
        // {RootedAT::QUERY_TREE, 2500},  //
        // {RootedAT::TREE_SIZE, 1000},   //
    };

    const int N = 300;
    slow_tree<true> slow(N);
    depth_first_tree<dft_node_sum<int64_t>> tree(N);
    auto actions = make_rooted_actions(N, 2s, stress_actions, 9 * N / 10);
    bool ok = true;

    for (int ia = 0, A = actions.size(); ia < A; ia++) {
        print_regular(ia, A, 1000, "stress test dft tree");
        auto [action, u, v, r, who, val] = actions[ia];

        switch (action) {
        case RootedAT::LINK: {
            slow.link(u, v);
            tree.link(u, v);
        } break;
        case RootedAT::CUT: {
            slow.cut(u);
            tree.cut(u);
        } break;
        case RootedAT::FINDROOT: {
            int ans = tree.findroot(u);
            ok = ans == who;
        } break;
        case RootedAT::QUERY_NODE: {
            long ans = tree.access_node(u)->self;
            ok = val == ans;
        } break;
        case RootedAT::UPDATE_NODE: {
            slow.update_node(u, val);
            tree.access_node(u)->self = val;
        } break;
        case RootedAT::UPDATE_SUBTREE: {
            slow.update_subtree(u, val);
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

    assert(ok);
}

int main() {
    RUN_BLOCK(stress_test_depth_first_tree());
    return 0;
}
