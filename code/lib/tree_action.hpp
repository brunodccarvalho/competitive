#pragma once

#include "lib/slow_tree.hpp"
#include "lib/test_chrono.hpp"
#include "lib/test_progress.hpp"

/**
 * For battle testing link cut trees, euler tour trees and top trees.
 * Tree may be rooted (ETT) or unrooted (LCT). The semantics of the actions changes.
 */
namespace tree_testing {

template <typename Type>
using actions_t = map<Type, int>;

enum class UnrootedAT : int {
    // TOPOLOGY UPDATES
    LINK,     // pick any u,v in distinct trees --> link(u,v)
    CUT,      // pick any non-root u with parent v --> cut(u,v) or cut(v,u)
    LINK_CUT, // pick any u, make LINK or CUT (HIDDEN)

    // TOPOLOGY QUERIES
    FINDROOT, // pick any u, reroot u's tree on a random who --> reroot(who), findroot(u)
    LCA,      // pick any u,v and r in u's tree --> reroot(r), lca(u,v) -> who
    LCA_CONN, // pick any u,v,r in the same tree --> LCA (HIDDEN)

    // NODE QUERIES
    QUERY_NODE,  // pick any u --> query_node(u) -> val
    UPDATE_NODE, // pick any u --> update_node(u,val)

    // PATH QUERIES
    QUERY_PATH,  // pick any connected u,v --> query_path(u,v) -> val
    PATH_LENGTH, // pick any connected u,v --> path_length(u,v) -> val
    UPDATE_PATH, // pick any connected u,v --> update_path(u,v,val)

    // SUBTREE QUERIES
    QUERY_SUBTREE,  // pick any connected u,v --> query_subtree(u,v) -> val (u below v)
    SUBTREE_SIZE,   // pick any connected u,v --> subtree_size(u,v) -> val (u below v)
    UPDATE_SUBTREE, // pick any connected u,v --> update_subtree(u,v,val) (u below v)

    // TREE QUERIES
    QUERY_TREE,  // pick any u --> reroot(u), query_subtree(u) -> val
    TREE_SIZE,   // pick any u --> reroot(u), subtree_size(u) -> val
    UPDATE_TREE, // pick any u --> reroot(u), update_subtree(u,val)

    STRESS_TEST,
    END,
};

template <typename AT>
const map<AT, string> action_names;

template <>
const map<UnrootedAT, string> action_names<UnrootedAT> = {
    {UnrootedAT::LINK, "LINK"},
    {UnrootedAT::CUT, "CUT"},
    {UnrootedAT::LINK_CUT, "LINK_CUT"},
    {UnrootedAT::FINDROOT, "FINDROOT"},
    {UnrootedAT::LCA, "LCA"},
    {UnrootedAT::LCA_CONN, "LCA_CONN"},
    {UnrootedAT::QUERY_NODE, "QUERY_NODE"},
    {UnrootedAT::UPDATE_NODE, "UPDATE_NODE"},
    {UnrootedAT::QUERY_PATH, "QUERY_PATH"},
    {UnrootedAT::UPDATE_PATH, "UPDATE_PATH"},
    {UnrootedAT::PATH_LENGTH, "PATH_LENGTH"},
    {UnrootedAT::QUERY_SUBTREE, "QUERY_SUBTREE"},
    {UnrootedAT::UPDATE_SUBTREE, "UPDATE_SUBTREE"},
    {UnrootedAT::SUBTREE_SIZE, "SUBTREE_SIZE"},
    {UnrootedAT::QUERY_TREE, "QUERY_TREE"},
    {UnrootedAT::UPDATE_TREE, "UPDATE_TREE"},
    {UnrootedAT::TREE_SIZE, "TREE_SIZE"},
    {UnrootedAT::STRESS_TEST, "STRESS_TEST"},
    {UnrootedAT::END, "END"},
};

enum class RootedAT : int {
    // TOPOLOGY UPDATES
    LINK,     // pick any root u, and v in a distinct tree --> link(u,v)
    CUT,      // pick any non-root u with parent v --> cut(u,v)
    LINK_CUT, // pick any u, make LINK or CUT (HIDDEN)
    REROOT,   // pick any u --> reroot(u)

    // TOPOLOGY QUERIES
    FINDROOT, // pick any u --> findroot(u)
    LCA,      // pick any u,v --> lca(u,v)
    LCA_CONN, // pick any connected u,v --> LCA (HIDDEN)

    // NODE QUERIES
    QUERY_NODE,  // pick any u --> query_node(u)
    UPDATE_NODE, // pick any u --> update_node(u,val)

    // PATH QUERIES
    QUERY_PATH,  // pick any connected u,v --> query_path(u,v)
    PATH_LENGTH, // pick any connected u,v --> path_length(u,v)
    UPDATE_PATH, // pick any connected u,v --> update_path(u,v,val)

    // SUBTREE QUERIES
    QUERY_SUBTREE,  // pick any u --> query_subtree(u)
    SUBTREE_SIZE,   // pick any u --> subtree_size(u)
    UPDATE_SUBTREE, // pick any u --> update_subtree(u,val)

    STRESS_TEST,
    END,
};

template <>
const map<RootedAT, string> action_names<RootedAT> = {
    {RootedAT::LINK, "LINK"},
    {RootedAT::CUT, "CUT"},
    {RootedAT::LINK_CUT, "LINK_CUT"},
    {RootedAT::REROOT, "REROOT"},
    {RootedAT::FINDROOT, "FINDROOT"},
    {RootedAT::LCA, "LCA"},
    {RootedAT::LCA_CONN, "LCA_CONN"},
    {RootedAT::QUERY_NODE, "QUERY_NODE"},
    {RootedAT::UPDATE_NODE, "UPDATE_NODE"},
    {RootedAT::QUERY_PATH, "QUERY_PATH"},
    {RootedAT::UPDATE_PATH, "UPDATE_PATH"},
    {RootedAT::PATH_LENGTH, "PATH_LENGTH"},
    {RootedAT::QUERY_SUBTREE, "QUERY_SUBTREE"},
    {RootedAT::UPDATE_SUBTREE, "UPDATE_SUBTREE"},
    {RootedAT::SUBTREE_SIZE, "SUBTREE_SIZE"},
    {RootedAT::STRESS_TEST, "STRESS_TEST"},
    {RootedAT::END, "END"},
};

template <typename Type>
auto make_tree_action_selector(const actions_t<Type>& freq) {
    vector<int> arr(int(Type::END) + 1);
    for (auto [key, proportion] : freq) {
        arr[int(key)] = proportion;
    }
    return discrete_distribution(begin(arr), end(arr));
}

template <typename History>
auto get_occurrences(const History& history) {
    using AT = decltype(history[0].action);
    vector<int> occ(int(AT::END) + 1);
    for (int i = 0, H = history.size(); i < H; i++) {
        occ[int(history[i].action)]++;
    }
    return occ;
}

template <typename History>
void print_occurrences(const History& history) {
    using AT = decltype(history[0].action);
    auto occ = get_occurrences(history);
    for (int i = 0, F = occ.size(); i < F; i++) {
        if (occ[i] > 0) {
            print("\t{}: {}\n", action_names<AT>.at(AT(i)), occ[i]);
        }
    }
}

template <typename Type>
struct TreeAction {
    Type action = Type::END;
    int u, v, r, w;
    long value;

  private:
    TreeAction(Type a, int u = 0, int v = 0, int r = 0, int w = 0, long value = 0)
        : action(a), u(u), v(v), r(r), w(w), value(value) {}

  public:
    static auto simple(Type a) { return TreeAction(a); }
    static auto au(Type a, int u) { return TreeAction(a, u); }
    static auto uv(Type a, int u, int v) { return TreeAction(a, u, v); }
    static auto uvr(Type a, int u, int v, int r) { return TreeAction(a, u, v, r); }
    static auto who(Type a, int u, int who) { return TreeAction(a, u, 0, 0, who); }
    static auto who(Type a, int u, int v, int who) { return TreeAction(a, u, v, 0, who); }
    static auto who(Type a, int u, int v, int r, int who) {
        return TreeAction(a, u, v, r, who);
    }
    static auto val(Type a, int u, long val) { return TreeAction(a, u, 0, 0, 0, val); }
    static auto val(Type a, int u, int v, long val) {
        return TreeAction(a, u, v, 0, 0, val);
    }
};

auto make_unrooted_actions(int N, ms runtime, const actions_t<UnrootedAT>& freq,
                           int initial_links = 0, bool subtree_only_edges = false) {
    constexpr long minvalue = 1, maxvalue = 500;
    constexpr long mindelta = -200, maxdelta = 200;
    intd noded(1, N);
    longd vald(minvalue, maxvalue);
    longd deltad(mindelta, maxdelta);
    boold coind(0.5);

    slow_tree<false> slow(N);
    auto selector = make_tree_action_selector(freq);

    using Action = TreeAction<UnrootedAT>;
    vector<Action> history;
    size_t size_sum = 0;

    initial_links = min(initial_links, N - 1);
    while (initial_links--) {
        auto [u, v] = slow.random_unconnected();
        if (coind(mt))
            swap(u, v);
        slow.link(u, v);
        history.emplace_back(Action::uv(UnrootedAT::LINK, u, v));
    }

    auto subtree_link = [&]() {
        if (subtree_only_edges) {
            return slow.random_edge();
        } else {
            return slow.random_connected();
        }
    };

    LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
        print_time(now, runtime, "preparing history (T={})...", slow.num_trees());

        auto action = UnrootedAT(selector(mt));

        switch (action) {
        case UnrootedAT::LINK: {
            if (slow.num_trees() > 1) {
                auto [u, v] = slow.random_unconnected();
                if (coind(mt))
                    swap(u, v);
                slow.link(u, v);
                history.emplace_back(Action::uv(UnrootedAT::LINK, u, v));
            }
        } break;
        case UnrootedAT::CUT: {
            if (slow.num_trees() < N) {
                int u = slow.random_non_root();
                int v = slow.parent[u];
                if (coind(mt))
                    swap(u, v);
                slow.cut(u, v);
                history.emplace_back(Action::uv(UnrootedAT::CUT, u, v));
            }
        } break;
        case UnrootedAT::LINK_CUT: {
            int u = noded(mt);
            if (int v = slow.parent[u]; v) {
                if (coind(mt))
                    swap(u, v);
                slow.cut(u, v);
                history.emplace_back(Action::uv(UnrootedAT::CUT, u, v));
            } else if (slow.num_trees() > 1) {
                v = slow.random_unconnected(u);
                if (coind(mt))
                    swap(u, v);
                slow.link(u, v);
                history.emplace_back(Action::uv(UnrootedAT::LINK, u, v));
            }
        } break;
        case UnrootedAT::FINDROOT: {
            int u = noded(mt);
            int who = slow.findroot(u);
            history.emplace_back(Action::who(UnrootedAT::FINDROOT, u, who));
        } break;
        case UnrootedAT::LCA: {
            auto [u, r] = slow.random_connected();
            int v = noded(mt);
            slow.reroot(r);
            int who = slow.lca(u, v);
            history.emplace_back(Action::who(UnrootedAT::LCA, u, v, r, who));
        } break;
        case UnrootedAT::LCA_CONN: {
            auto [u, r] = slow.random_connected();
            slow.reroot(r);
            int v = slow.random_in_subtree(r);
            int who = slow.lca(u, v);
            history.emplace_back(Action::who(UnrootedAT::LCA, u, v, r, who));
        } break;
        case UnrootedAT::QUERY_NODE: {
            int u = noded(mt);
            long val = slow.query_node(u);
            history.emplace_back(Action::val(UnrootedAT::QUERY_NODE, u, val));
        } break;
        case UnrootedAT::UPDATE_NODE: {
            int u = noded(mt);
            long val = vald(mt);
            slow.update_node(u, val);
            history.emplace_back(Action::val(UnrootedAT::UPDATE_NODE, u, val));
        } break;
        case UnrootedAT::QUERY_PATH: {
            auto [u, v] = slow.random_connected();
            long val = slow.query_path(u, v);
            history.emplace_back(Action::val(UnrootedAT::QUERY_PATH, u, v, val));
        } break;
        case UnrootedAT::PATH_LENGTH: {
            auto [u, v] = slow.random_connected();
            long val = slow.path_length(u, v);
            history.emplace_back(Action::val(UnrootedAT::PATH_LENGTH, u, v, val));
        } break;
        case UnrootedAT::UPDATE_PATH: {
            auto [u, v] = slow.random_connected();
            long val = deltad(mt);
            slow.update_path(u, v, val);
            history.emplace_back(Action::val(UnrootedAT::UPDATE_PATH, u, v, val));
        } break;
        case UnrootedAT::QUERY_SUBTREE: {
            auto [u, v] = subtree_link();
            long val = slow.query_subtree(u, v);
            history.emplace_back(Action::val(UnrootedAT::QUERY_SUBTREE, u, v, val));
        } break;
        case UnrootedAT::SUBTREE_SIZE: {
            auto [u, v] = subtree_link();
            long val = slow.subtree_size(u, v);
            history.emplace_back(Action::val(UnrootedAT::SUBTREE_SIZE, u, v, val));
        } break;
        case UnrootedAT::UPDATE_SUBTREE: {
            auto [u, v] = subtree_link();
            long val = deltad(mt);
            slow.update_subtree(u, v, val);
            history.emplace_back(Action::val(UnrootedAT::UPDATE_SUBTREE, u, v, val));
        } break;
        case UnrootedAT::QUERY_TREE: {
            int u = noded(mt);
            long val = slow.query_tree(u);
            history.emplace_back(Action::val(UnrootedAT::QUERY_TREE, u, val));
        } break;
        case UnrootedAT::TREE_SIZE: {
            int u = noded(mt);
            slow.reroot(u);
            long val = slow.tree_size(u);
            history.emplace_back(Action::val(UnrootedAT::TREE_SIZE, u, val));
        } break;
        case UnrootedAT::UPDATE_TREE: {
            int u = noded(mt);
            long val = deltad(mt);
            slow.update_tree(u, val);
            history.emplace_back(Action::val(UnrootedAT::UPDATE_TREE, u, val));
        } break;
        case UnrootedAT::STRESS_TEST: {
            history.emplace_back(Action::simple(UnrootedAT::STRESS_TEST));
        } break;
        default:
            throw runtime_error("Unknown action");
        }

        size_sum += slow.num_trees();
    }

    printcl("S avg: {:.2f}\n", 1.0 * size_sum / runs);
    printcl("runs: {}\n", runs);
    return history;
}

auto make_rooted_actions(int N, ms runtime, const actions_t<RootedAT>& freq,
                         int initial_links = 0) {
    constexpr long minvalue = 1, maxvalue = 500;
    constexpr long mindelta = -200, maxdelta = 200;
    intd noded(1, N);
    longd vald(minvalue, maxvalue);
    longd deltad(mindelta, maxdelta);

    slow_tree<true> slow(N);
    auto selector = make_tree_action_selector(freq);

    using Action = TreeAction<RootedAT>;
    vector<Action> history;
    size_t size_sum = 0;

    initial_links = min(initial_links, N - 1);
    while (initial_links--) {
        auto [u, v] = slow.random_unconnected();
        u = slow.findroot(u);
        slow.link(u, v);
        history.emplace_back(Action::uv(RootedAT::LINK, u, v));
    }

    LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
        print_time(now, runtime, "preparing history (T={})...", slow.num_trees());

        auto action = RootedAT(selector(mt));

        switch (action) {
        case RootedAT::LINK: {
            if (slow.num_trees() > 1) {
                auto [u, v] = slow.random_unconnected();
                u = slow.findroot(u);
                slow.link(u, v);
                history.emplace_back(Action::uv(RootedAT::LINK, u, v));
            }
        } break;
        case RootedAT::CUT: {
            if (slow.num_trees() < N) {
                int u = slow.random_non_root();
                slow.cut(u);
                history.emplace_back(Action::au(RootedAT::CUT, u));
            }
        } break;
        case RootedAT::LINK_CUT: {
            int u = noded(mt);
            if (int v = slow.parent[u]; v) {
                slow.cut(u);
                history.emplace_back(Action::au(RootedAT::CUT, u));
            } else if (slow.num_trees() > 1) {
                v = slow.random_unconnected(u);
                slow.link(u, v);
                history.emplace_back(Action::uv(RootedAT::LINK, u, v));
            }
        } break;
        case RootedAT::REROOT: {
            int u = noded(mt);
            slow.reroot(u);
            history.emplace_back(Action::au(RootedAT::REROOT, u));
        } break;
        case RootedAT::FINDROOT: {
            int u = noded(mt);
            int who = slow.findroot(u);
            history.emplace_back(Action::who(RootedAT::FINDROOT, u, who));
        } break;
        case RootedAT::LCA: {
            int u = noded(mt);
            int v = noded(mt);
            int who = slow.lca(u, v);
            history.emplace_back(Action::who(RootedAT::LCA, u, v, who));
        } break;
        case RootedAT::LCA_CONN: {
            auto [u, v] = slow.random_connected();
            int who = slow.lca(u, v);
            history.emplace_back(Action::who(RootedAT::LCA, u, v, who));
        } break;
        case RootedAT::QUERY_NODE: {
            int u = noded(mt);
            long val = slow.query_node(u);
            history.emplace_back(Action::val(RootedAT::QUERY_NODE, u, val));
        } break;
        case RootedAT::UPDATE_NODE: {
            int u = noded(mt);
            long val = vald(mt);
            slow.update_node(u, val);
            history.emplace_back(Action::val(RootedAT::UPDATE_NODE, u, val));
        } break;
        case RootedAT::QUERY_PATH: {
            auto [u, v] = slow.random_connected();
            long val = slow.query_path(u, v);
            history.emplace_back(Action::val(RootedAT::QUERY_PATH, u, v, val));
        } break;
        case RootedAT::PATH_LENGTH: {
            auto [u, v] = slow.random_connected();
            long val = slow.path_length(u, v);
            history.emplace_back(Action::val(RootedAT::PATH_LENGTH, u, v, val));
        } break;
        case RootedAT::UPDATE_PATH: {
            auto [u, v] = slow.random_connected();
            long val = deltad(mt);
            slow.update_path(u, v, val);
            history.emplace_back(Action::val(RootedAT::UPDATE_PATH, u, v, val));
        } break;
        case RootedAT::QUERY_SUBTREE: {
            int u = noded(mt);
            long val = slow.query_subtree(u);
            history.emplace_back(Action::val(RootedAT::QUERY_SUBTREE, u, val));
        } break;
        case RootedAT::SUBTREE_SIZE: {
            int u = noded(mt);
            long val = slow.subtree_size(u);
            history.emplace_back(Action::val(RootedAT::SUBTREE_SIZE, u, val));
        } break;
        case RootedAT::UPDATE_SUBTREE: {
            int u = noded(mt);
            long val = deltad(mt);
            slow.update_subtree(u, val);
            history.emplace_back(Action::val(RootedAT::UPDATE_SUBTREE, u, val));
        } break;
        case RootedAT::STRESS_TEST: {
            history.emplace_back(Action::simple(RootedAT::STRESS_TEST));
        } break;
        default:
            throw runtime_error("Unknown action");
        }

        size_sum += slow.num_trees();
    }

    printcl("S avg: {:8.2f} actions: {}\n", 1.0 * size_sum / runs, history.size());
    return history;
}

} // namespace tree_testing
