#include "test_utils.hpp"
#include "../struct/top_tree.hpp"
#include "../numeric/modnum.hpp"
#include "../struct/hld_forest.hpp"
#include "../struct/segtree.hpp"
#include "../struct/segtree_nodes.hpp"
#include "../struct/segtree_nodes.hpp"
#include "../lib/graph_generator.hpp"
#include "../lib/tree_action.hpp"

using namespace tree_testing;

void stress_test_top_tree() {
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

        {UnrootedAT::UPDATE_SUBTREE, 3500}, //
        {UnrootedAT::QUERY_SUBTREE, 3500},  //
        {UnrootedAT::SUBTREE_SIZE, 1500},   //

        {UnrootedAT::UPDATE_TREE, 2500}, //
        {UnrootedAT::QUERY_TREE, 2500},  //
        {UnrootedAT::TREE_SIZE, 1000},   //
    };

    const int N = 300;
    slow_tree<false> slow(N);
    top_tree<top_sum_node> tree(N);
    auto actions = make_unrooted_actions(N, 2s, stress_actions, 9 * N / 10);
    bool ok = true;

    for (int ia = 0, A = actions.size(); ia < A; ia++) {
        print_regular(ia, A, 1000, "stress test top tree");
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
            int64_t ans = tree.access_node(u)->self;
            ok = val == ans;
        } break;
        case UnrootedAT::UPDATE_NODE: {
            slow.update_node(u, val);
            tree.access_node(u)->self = val;
        } break;
        case UnrootedAT::UPDATE_PATH: {
            slow.update_path(u, v, val);
            tree.access_path(u, v)->add_path(val);
        } break;
        case UnrootedAT::QUERY_PATH: {
            int64_t ans = tree.access_path(u, v)->path;
            ok = val == ans;
        } break;
        case UnrootedAT::PATH_LENGTH: {
            int64_t ans = tree.access_path(u, v)->path_size;
            ok = val == ans;
        } break;
        case UnrootedAT::UPDATE_SUBTREE: {
            slow.update_subtree(u, v, val);
            tree.access_subtree(u, v)->add_virt(val);
        } break;
        case UnrootedAT::QUERY_SUBTREE: {
            int64_t ans = tree.access_subtree(u, v)->virt;
            ok = val == ans;
        } break;
        case UnrootedAT::SUBTREE_SIZE: {
            int ans = tree.access_subtree(u, v)->virt_size;
            ok = val == ans;
        } break;
        case UnrootedAT::UPDATE_TREE: {
            slow.reroot(u);
            slow.update_tree(u, val);
            tree.access_tree(u)->add_virt(val);
        } break;
        case UnrootedAT::QUERY_TREE: {
            int64_t ans = tree.access_tree(u)->virt;
            ok = val == ans;
        } break;
        case UnrootedAT::TREE_SIZE: {
            int ans = tree.access_tree(u)->virt_size;
            assert(val == slow.tree_size(u));
            ok = val == ans;
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
    RUN_BLOCK(stress_test_top_tree());
    return 0;
}
