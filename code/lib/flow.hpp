#pragma once

#include "random.hpp"

template <typename Flow>
auto generate_feasible_circulation(int V, const vector<array<int, 2>>& g,
                                   array<Flow, 2> flowrange, int repulsion = -10) {
    int E = g.size();
    auto flow1 = rands_wide<Flow>(E, flowrange[0], flowrange[1], repulsion);
    auto flow2 = rands_wide<Flow>(E, flowrange[0], flowrange[1], repulsion);
    auto flow3 = rands_wide<Flow>(E, flowrange[0], flowrange[1], repulsion);
    vector<Flow> lower(E), upper(E), flow(E), supply(V);
    for (int e = 0; e < E; e++) {
        lower[e] = min({flow1[e], flow2[e], flow3[e]});
        upper[e] = max({flow1[e], flow2[e], flow3[e]});
        flow[e] = lower[e] ^ upper[e] ^ flow1[e] ^ flow2[e] ^ flow3[e];
        auto [u, v] = g[e];
        supply[u] += flow[e];
        supply[v] -= flow[e];
    }
    return make_tuple(move(lower), move(upper), move(flow), move(supply));
}
