#include "test_utils.hpp"
#include "matching/stable_matching.hpp"

void unit_test_stable_matching() {
    vector<vector<int>> up, vp;
    up = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2}};
    vp = {{0, 1, 2}, {2, 0, 1}, {2, 1, 0}};

    auto [mu, mv] = stable_matching(up, vp);

    for (int u = 0, V = mu.size(); u < V; u++) {
        print("{} -- {}\n", u, mu[u]);
    }
}

int main() {
    RUN_BLOCK(unit_test_stable_matching());
    return 0;
}
