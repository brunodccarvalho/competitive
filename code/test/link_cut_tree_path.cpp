#include "test_utils.hpp"
#include "struct/link_cut_tree_path.hpp"
#include "lib/slow_tree.hpp"
#include "lib/tree_action.hpp"

using namespace tree_testing;

void stress_test_link_cut_tree_path() {
    static actions_t<UnrootedAT> stress_actions = {
        {UnrootedAT::LINK, 1500},     //
        {UnrootedAT::CUT, 300},       //
        {UnrootedAT::LINK_CUT, 1200}, //
        {UnrootedAT::FINDROOT, 1000}, //
        {UnrootedAT::LCA, 500},       //
        {UnrootedAT::LCA_CONN, 1000}, //

        {UnrootedAT::UPDATE_NODE, 1500}, //
        {UnrootedAT::QUERY_NODE, 2000},  //

        {UnrootedAT::UPDATE_PATH, 3500}, //
        {UnrootedAT::QUERY_PATH, 3500},  //
        {UnrootedAT::PATH_LENGTH, 1500}, //

        // {UnrootedAT::UPDATE_SUBTREE, 3500}, //
        // {UnrootedAT::QUERY_SUBTREE, 3500},  //
        // {UnrootedAT::SUBTREE_SIZE, 1500},   //

        // {UnrootedAT::UPDATE_TREE, 2500}, //
        // {UnrootedAT::QUERY_TREE, 2500},  //
        // {UnrootedAT::TREE_SIZE, 1000},   //
    };

    const int N = 300;
    slow_tree<false> slow(N);
    link_cut_tree_path<lct_node_path_sum> tree(N);
    auto actions = make_unrooted_actions(N, 2s, stress_actions, 9 * N / 10);
    bool ok = true;

    for (int ia = 0, A = actions.size(); ia < A; ia++) {
        print_regular(ia, A, 1000, "stress test lct path");
        auto [action, u, v, r, who, val] = actions[ia];

        switch (action) {
        case UnrootedAT::LINK: {
            slow.link(u, v);
            bool fine = tree.link(u, v);
            assert(fine);
        } break;
        case UnrootedAT::CUT: {
            slow.cut(u, v);
            bool fine = tree.cut(u, v);
            assert(fine);
        } break;
        case UnrootedAT::FINDROOT: {
            tree.reroot(who);
            slow.reroot(who);
            int ans = tree.findroot(u);
            ok = ans == who;
        } break;
        case UnrootedAT::LCA: {
            tree.reroot(r);
            slow.reroot(r);
            int ans = tree.lca(u, v);
            ok = ans == who;
        } break;
        case UnrootedAT::QUERY_NODE: {
            long ans = tree.access_node(u)->self;
            ok = val == ans;
        } break;
        case UnrootedAT::UPDATE_NODE: {
            slow.update_node(u, val);
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
            slow.update_path(u, v, val);
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

    assert(ok);
}

int main() {
    RUN_SHORT(stress_test_link_cut_tree_path());
    return 0;
}
