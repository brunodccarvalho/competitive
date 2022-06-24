#include "test_utils.hpp"
#include "struct/euler_tour_tree.hpp"
#include "lib/tree_action.hpp"

using namespace tree_testing;

void stress_test_euler_tour_tree() {
    static actions_t<UnrootedAT> stress_actions = {
        {UnrootedAT::LINK, 1500},     //
        {UnrootedAT::CUT, 300},       //
        {UnrootedAT::LINK_CUT, 1200}, //
        {UnrootedAT::FINDROOT, 1000}, //
        // {UnrootedAT::LCA, 500},       //
        // {UnrootedAT::LCA_CONN, 1000}, //

        {UnrootedAT::UPDATE_NODE, 1500}, //
        {UnrootedAT::QUERY_NODE, 2000},  //

        // {UnrootedAT::UPDATE_PATH, 3500}, //
        // {UnrootedAT::QUERY_PATH, 3500},  //
        // {UnrootedAT::PATH_LENGTH, 1500}, //

        {UnrootedAT::UPDATE_SUBTREE, 3500}, //
        {UnrootedAT::QUERY_SUBTREE, 3500},  //
        {UnrootedAT::SUBTREE_SIZE, 1500},   //

        {UnrootedAT::UPDATE_TREE, 2500}, //
        {UnrootedAT::QUERY_TREE, 2500},  //
        {UnrootedAT::TREE_SIZE, 1000},   //
    };

    const int N = 300;
    slow_tree<false> slow(N);
    euler_tour_tree<ett_node_sum<int64_t>> tree(N);
    auto actions = make_unrooted_actions(N, 2s, stress_actions, 9 * N / 10, true);
    bool ok = true;

    for (int ia = 0, A = actions.size(); ia < A; ia++) {
        print_regular(ia, A, 10, "stress test euler tour tree");
        auto [action, u, v, r, who, val] = actions[ia];

        switch (action) {
        case UnrootedAT::LINK: {
            slow.link(u, v);
            ok = tree.link(u, v);
        } break;
        case UnrootedAT::CUT: {
            slow.cut(u, v);
            ok = tree.cut(u, v);
        } break;
        case UnrootedAT::FINDROOT: {
            tree.reroot(who);
            slow.reroot(who);
            int ans = tree.findroot(u);
            ok = ans == who;
        } break;
        case UnrootedAT::QUERY_NODE: {
            int64_t ans = tree.access_node(u)->self;
            ok = val == ans;
        } break;
        case UnrootedAT::UPDATE_NODE: {
            slow.update_node(u, val);
            tree.access_node(u)->self = val;
        } break;
        case UnrootedAT::QUERY_TREE: {
            int64_t ans = tree.access_tree(u)->subtree();
            ok = val == ans;
        } break;
        case UnrootedAT::TREE_SIZE: {
            int ans = tree.access_tree(u)->subtree_size();
            ok = val == ans;
        } break;
        case UnrootedAT::UPDATE_TREE: {
            slow.reroot(u);
            slow.update_tree(u, val);
            tree.access_tree(u)->lazy += val;
        } break;
        case UnrootedAT::QUERY_SUBTREE: {
            int64_t ans = tree.access_subtree(u, v)->subtree();
            tree.end_access();
            ok = val == ans;
        } break;
        case UnrootedAT::SUBTREE_SIZE: {
            int ans = tree.access_subtree(u, v)->subtree_size();
            tree.end_access();
            ok = val == ans;
        } break;
        case UnrootedAT::UPDATE_SUBTREE: {
            slow.update_subtree(u, v, val);
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
}

int main() {
    RUN_BLOCK(stress_test_euler_tour_tree());
    return 0;
}
