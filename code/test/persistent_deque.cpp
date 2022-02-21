#include "test_utils.hpp"
#include "struct/persistent_deque.hpp"

void stress_test_persistent_jump_deque() {
    vector<deque<int>> deques(1);
    persistent_jump_deque<int> pd;
    int V = pd.versions();
    assert(V == 1);

    boold coind(0.5);
    mt.seed(73);

    LOOP_FOR_DURATION_OR_RUNS (1s, 150000) {
        if (coind(mt)) { // push element on the front
            int v = rand_wide<int>(0, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            int v0 = pd.push_front(v, n);

            int v1 = deques.size();
            deques.push_back(deques[v]);
            deques.back().push_front(n);

            assert(V == v0 && V == v1);
            assert(pd.front(V) == deques[V].front());
            V++;
        }
        if (coind(mt)) { // push element on the back
            int v = rand_wide<int>(0, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            int v0 = pd.push_back(v, n);

            int v1 = deques.size();
            deques.push_back(deques[v]);
            deques.back().push_back(n);

            assert(V == v0 && V == v1);
            assert(pd.back(V) == deques[V].back());
            V++;
        }
        if (coind(mt)) { // pop element from the front
            int v = rand_unif<int>(0, V - 1);
            assert(int(deques[v].size()) == pd.size(v));

            int v0 = pd.pop_front(v);

            int v1 = deques.size();
            deques.push_back(deques[v]);
            if (!deques.back().empty()) {
                deques.back().pop_front();
            }

            assert(V == v0 && V == v1);
            assert(deques[V].empty() == pd.empty(V));
            assert(deques[V].empty() || pd.front(V) == deques[V].front());
            V++;
        }
        if (coind(mt)) { // pop element from the back
            int v = rand_unif<int>(0, V - 1);
            assert(int(deques[v].size()) == pd.size(v));

            int v0 = pd.pop_back(v);

            int v1 = deques.size();
            deques.push_back(deques[v]);
            if (!deques.back().empty()) {
                deques.back().pop_back();
            }

            assert(V == v0 && V == v1);
            assert(deques[V].empty() == pd.empty(V));
            assert(deques[V].empty() || pd.back(V) == deques[V].back());
            V++;
        }
        if (coind(mt) && V > 1) { // push inplace front
            int v = rand_wide<int>(1, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            pd.push_front_inplace(v, n);

            deques[v].push_front(n);

            assert(pd.front(v) == deques[v].front());
        }
        if (coind(mt) && V > 1) { // push inplace back
            int v = rand_wide<int>(1, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            pd.push_back_inplace(v, n);

            deques[v].push_back(n);

            assert(pd.back(v) == deques[v].back());
        }
        if (coind(mt) && V > 1) { // pop element inplace from front
            int v = rand_unif<int>(1, V - 1);
            assert(int(deques[v].size()) == pd.size(v));

            pd.pop_front_inplace(v);

            if (!deques[v].empty()) {
                deques[v].pop_front();
            }

            assert(deques[v].empty() || pd.front(v) == deques[v].front());
        }
        if (coind(mt) && V > 1) { // pop element inplace from back
            int v = rand_unif<int>(1, V - 1);
            assert(int(deques[v].size()) == pd.size(v));

            pd.pop_back_inplace(v);

            if (!deques[v].empty()) {
                deques[v].pop_back();
            }

            assert(deques[v].empty() || pd.back(v) == deques[v].back());
        }
        if (coind(mt)) { // verify elements in a random version
            int v = rand_unif<int>(0, V - 1);

            int S = pd.size(v);
            int S_actual = deques[v].size();
            assert(S == S_actual);

            for (int i = 0; i < S; i++) {
                assert(pd.find_from_front(v, i) == deques[v][i]);
                assert(pd.find_from_back(v, i) == deques[v][S - i - 1]);
            }
        }
    }

    int S = 0, M = 0;
    for (int i = 0; i < V; i++) {
        S += deques[i].size();
        M = max<int>(M, deques[i].size());
    }
    println("final versions: {}", V);
    println("final total size: {}", S);
    println("final max size: {}", M);
}

int main() {
    RUN_BLOCK(stress_test_persistent_jump_deque());
    return 0;
}
