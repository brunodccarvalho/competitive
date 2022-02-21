#include "test_utils.hpp"
#include "lib/graph_formats.hpp"
#include "graphs/dominator_tree.hpp"

void unit_test_dominator_tree() {
    edges_t g;
    int V;
    enum nodenames { _, R = 1, A, B, C, D, E, F, G, H, I = 10, J, K, L, M, N, O, P, Q };
    char name[] = "_RABCDEFGHIJKLMNOPQ";

    auto run = [&]() {
        auto out = make_adjacency_lists_directed(V, g);
        auto [dom, parent] = build_dominator_tree(R, out);
        for (int u = 1; u < V; u++) {
            print("dom[{}] = {}  parent[{}] = {}\n", name[u], name[dom[u]], name[u],
                  name[parent[u]]);
        }
        print("---\n");
    };

    V = 14;
    g = {
        {R, A}, {R, B}, {R, C}, {A, D}, {B, A}, {B, D}, {B, E},
        {C, F}, {C, G}, {D, L}, {E, H}, {F, I}, {G, I}, {G, J},
        {H, E}, {H, K}, {I, K}, {J, I}, {K, I}, {K, R}, {L, H},
    };
    run();

    V = 18;
    g = {
        {R, A}, {R, I}, {A, B}, {A, C}, {A, H}, {I, B}, {J, A}, {B, C},
        {H, J}, {H, C}, {C, D}, {C, K}, {C, I}, {K, L}, {D, M}, {D, E},
        {D, H}, {L, D}, {L, K}, {L, E}, {L, P}, {M, D}, {M, N}, {E, F},
        {N, E}, {N, O}, {F, N}, {F, L}, {F, P}, {F, G}, {P, G}, {G, O},
    };
    run();
}

int main() {
    RUN_SHORT(unit_test_dominator_tree());
    return 0;
}
