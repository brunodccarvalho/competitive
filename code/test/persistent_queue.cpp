#include "test_utils.hpp"
#include "struct/persistent_queue.hpp"

void stress_test_persistent_queue_stacks() {
    vector<deque<int>> deques(1);
    persistent_queue_stacks<int> pq;
    int V = pq.versions();

    boold coind(0.5);

    LOOP_FOR_DURATION_OR_RUNS (1s, 150000) {
        if (coind(mt)) { // push element
            int v = rand_wide<int>(0, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            int v0 = pq.push(v, n);

            int v1 = deques.size();
            deques.push_back(deques[v]);
            deques.back().push_back(n);

            assert(V == v0 && V == v1);
            assert(pq.front(V) == deques[V].front());
            V++;
        }
        if (coind(mt)) { // pop element
            int v = rand_unif<int>(0, V - 1);
            assert(int(deques[v].size()) == pq.size(v));

            int v0 = pq.pop(v);

            int v1 = deques.size();
            deques.push_back(deques[v]);
            if (!deques.back().empty()) {
                deques.back().pop_front();
            }

            assert(V == v0 && V == v1);
            assert(deques[V].empty() || pq.front(V) == deques[V].front());
            V++;
        }
        if (coind(mt) && V > 1) { // push inplace
            int v = rand_wide<int>(1, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            pq.push_inplace(v, n);

            deques[v].push_back(n);

            assert(pq.front(v) == deques[v].front());
        }
        if (coind(mt) && V > 1) { // pop element inplace
            int v = rand_unif<int>(1, V - 1);
            assert(int(deques[v].size()) == pq.size(v));

            pq.pop_inplace(v);

            if (!deques[v].empty()) {
                deques[v].pop_front();
            }

            assert(deques[v].empty() || pq.front(v) == deques[v].front());
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

void stress_test_persistent_jump_queue() {
    vector<deque<int>> deques(1);
    persistent_jump_queue<int> pq;
    int V = pq.versions();

    boold coind(0.5);
    mt.seed(73);

    LOOP_FOR_DURATION_OR_RUNS (1s, 150000) {
        if (coind(mt)) { // push element
            int v = rand_wide<int>(0, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            int v0 = pq.push(v, n);

            int v1 = deques.size();
            deques.push_back(deques[v]);
            deques.back().push_back(n);

            assert(V == v0 && V == v1);
            assert(pq.front(V) == deques[V].front());
            V++;
        }
        if (coind(mt)) { // pop element
            int v = rand_unif<int>(0, V - 1);
            assert(int(deques[v].size()) == pq.size(v));

            int v0 = pq.pop(v);

            int v1 = deques.size();
            deques.push_back(deques[v]);
            if (!deques.back().empty()) {
                deques.back().pop_front();
            }

            assert(V == v0 && V == v1);
            assert(deques[V].empty() == pq.empty(V));
            assert(deques[V].empty() || pq.front(V) == deques[V].front());
            V++;
        }
        if (coind(mt) && V > 1) { // push inplace
            int v = rand_wide<int>(1, V - 1, +3);
            int n = rand_unif<int>(-1000, 1000);

            pq.push_inplace(v, n);

            deques[v].push_back(n);

            assert(pq.front(v) == deques[v].front());
        }
        if (coind(mt) && V > 1) { // pop element inplace
            int v = rand_unif<int>(1, V - 1);
            assert(int(deques[v].size()) == pq.size(v));

            pq.pop_inplace(v);

            if (!deques[v].empty()) {
                deques[v].pop_front();
            }

            assert(deques[v].empty() || pq.front(v) == deques[v].front());
        }
        if (1) { // verify elements in a random version
            int v = rand_unif<int>(0, V - 1);

            int S = pq.size(v);
            int S_actual = deques[v].size();
            assert(S == S_actual);

            for (int i = 0; i < S; i++) {
                assert(pq.find_from_front(v, i) == deques[v][i]);
                assert(pq.find_from_back(v, i) == deques[v][S - i - 1]);
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
    RUN_BLOCK(stress_test_persistent_queue_stacks());
    RUN_BLOCK(stress_test_persistent_jump_queue());
    return 0;
}
