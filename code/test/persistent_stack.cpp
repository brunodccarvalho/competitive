#include "test_utils.hpp"
#include "struct/persistent_stack.hpp"

void stress_test_persistent_stack() {
    vector<vector<int>> stacks(1);
    persistent_stack<int> ps;
    int V = ps.versions();

    boold coind(0.5);

    LOOP_FOR_DURATION_OR_RUNS (1s, 150000) {
        if (coind(mt)) { // push element
            int v = rand_wide<int>(0, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            int v0 = ps.push(v, n);

            int v1 = stacks.size();
            stacks.push_back(stacks[v]);
            stacks.back().push_back(n);

            assert(V == v0 && V == v1);
            assert(ps.top(V) == stacks[V].back());
            V++;
        }
        if (coind(mt)) { // pop element
            int v = rand_unif<int>(0, V - 1);

            int v0 = ps.pop(v);

            int v1 = stacks.size();
            stacks.push_back(stacks[v]);
            if (!stacks.back().empty()) {
                stacks.back().pop_back();
            }

            assert(V == v0 && V == v1);
            assert(stacks[V].empty() || ps.top(V) == stacks[V].back());
            V++;
        }
        if (coind(mt) && V > 1) { // push inplace
            int v = rand_wide<int>(1, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            ps.push_inplace(v, n);

            stacks[v].push_back(n);

            assert(ps.top(v) == stacks[v].back());
        }
        if (coind(mt) && V > 1) { // pop element inplace
            int v = rand_unif<int>(1, V - 1);

            ps.pop_inplace(v);

            if (!stacks[v].empty()) {
                stacks[v].pop_back();
            }

            assert(stacks[v].empty() || ps.top(v) == stacks[v].back());
        }
    }

    int S = 0, M = 0;
    for (int i = 0; i < V; i++) {
        S += stacks[i].size();
        M = max<int>(M, stacks[i].size());
    }
    println("final versions: {}", V);
    println("final total size: {}", S);
    println("final max size: {}", M);
}

void stress_test_persistent_jump_stack() {
    vector<vector<int>> stacks(1);
    persistent_jump_stack<int> ps;
    int V = ps.versions();

    boold coind(0.5);

    LOOP_FOR_DURATION_OR_RUNS (1s, 150000) {
        if (coind(mt) && V > 1) { // query kth from the bottom
            int v = rand_unif<int>(1, V - 1);
            int S = ps.size(v);
            assert(S == int(stacks[v].size()));

            if (S > 0) {
                int k = rand_unif<int>(0, S - 1);

                int x = ps.find_from_bottom(v, k);
                int y = stacks[v][k];

                assert(x == y);
            }
        }
        if (coind(mt) && V > 1) { // query kth from the top
            int v = rand_unif<int>(1, V - 1);
            int S = ps.size(v);
            assert(S == int(stacks[v].size()));

            if (S > 0) {
                int k = rand_unif<int>(0, S - 1);

                int x = ps.find_from_top(v, k);
                int y = stacks[v][S - k - 1];

                assert(x == y);
            }
        }
        if (coind(mt)) { // push element
            int v = rand_wide<int>(0, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            int v0 = ps.push(v, n);

            int v1 = stacks.size();
            stacks.push_back(stacks[v]);
            stacks.back().push_back(n);

            assert(V == v0 && V == v1);
            assert(ps.top(V) == stacks[V].back());
            V++;
        }
        if (coind(mt)) { // pop element
            int v = rand_unif<int>(0, V - 1);
            assert(int(stacks[v].size()) == ps.size(v));

            int v0 = ps.pop(v);

            int v1 = stacks.size();
            stacks.push_back(stacks[v]);
            if (!stacks.back().empty()) {
                stacks.back().pop_back();
            }

            assert(V == v0 && V == v1);
            assert(stacks[V].empty() || ps.top(V) == stacks[V].back());
            V++;
        }
        if (coind(mt) && V > 1) { // push inplace
            int v = rand_wide<int>(1, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            ps.push_inplace(v, n);

            stacks[v].push_back(n);

            assert(ps.top(v) == stacks[v].back());
        }
        if (coind(mt) && V > 1) { // pop element inplace
            int v = rand_unif<int>(1, V - 1);

            ps.pop_inplace(v);

            if (!stacks[v].empty()) {
                stacks[v].pop_back();
            }

            assert(stacks[v].empty() || ps.top(v) == stacks[v].back());
        }
        if (true) { // verify elements in a random version
            int v = rand_unif<int>(0, V - 1);

            int S = ps.size(v);
            int S_actual = stacks[v].size();
            assert(S == S_actual);

            for (int i = 0; i < S; i++) {
                assert(ps.find_from_bottom(v, i) == stacks[v][i]);
                assert(ps.find_from_top(v, i) == stacks[v][S - i - 1]);
            }
        }
    }

    int S = 0, M = 0;
    for (int i = 0; i < V; i++) {
        S += stacks[i].size();
        M = max<int>(M, stacks[i].size());
    }
    println("final versions: {}", V);
    println("final total size: {}", S);
    println("final max size: {}", M);
}

int main() {
    RUN_BLOCK(stress_test_persistent_stack());
    RUN_BLOCK(stress_test_persistent_jump_stack());
    return 0;
}
