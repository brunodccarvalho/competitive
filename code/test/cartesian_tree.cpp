#include "test_utils.hpp"
#include "algo/cartesian_tree.hpp"
#include "algo/y_combinator.hpp"

auto stress_test_cartesian_tree() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (3s, now, 10000, runs) {
        print_time(now, 3s, "stress cartesian tree ({} runs)", runs);

        int N = rand_unif<int>(1, 1000);
        vector<int> index(N);
        iota(begin(index), end(index), 0);
        vector<int> arr = index;
        shuffle(begin(arr), end(arr), mt);

        auto [root, parent, kids] = cartesian_tree(arr);
        vector<int> inorder;

        y_combinator([&, kids = kids, parent = parent](auto self, int i) -> void {
            if (kids[i][0] != -1) {
                self(kids[i][0]);
                assert(kids[i][0] < i);
                assert(parent[kids[i][0]] == i);
                assert(arr[i] < arr[kids[i][0]]);
            }
            inorder.push_back(i);
            if (kids[i][1] != -1) {
                self(kids[i][1]);
                assert(i < kids[i][1]);
                assert(parent[kids[i][1]] == i);
                assert(arr[i] < arr[kids[i][1]]);
            }
        })(root);

        assert(inorder == index);
    }
}

int main() {
    RUN_BLOCK(stress_test_cartesian_tree());
    return 0;
}
