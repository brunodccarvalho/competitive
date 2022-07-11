#include "test_utils.hpp"
#include "../numeric/convolution.hpp"
#include "../algo/optimization.hpp"

auto naive_min_plus_tracked(const vector<int>& a, const vector<int>& b) {
    int N = a.size(), M = b.size();
    assert(N && M);
    vector<int> c(N + M - 1, INT_MAX);
    vector<int> whoa(N + M - 1), whob(N + M - 1);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (c[i + j] > a[i] + b[j]) {
                c[i + j] = a[i] + b[j];
                whoa[i + j] = i;
                whob[i + j] = j;
            }
        }
    }
    return make_tuple(c, whoa, whob);
}

auto naive_max_plus_tracked(const vector<int>& a, const vector<int>& b) {
    int N = a.size(), M = b.size();
    assert(N && M);
    vector<int> c(N + M - 1, INT_MIN);
    vector<int> whoa(N + M - 1), whob(N + M - 1);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (c[i + j] < a[i] + b[j]) {
                c[i + j] = a[i] + b[j];
                whoa[i + j] = i;
                whob[i + j] = j;
            }
        }
    }
    return make_tuple(c, whoa, whob);
}

auto naive_min_plus(const vector<int>& a, const vector<int>& b) {
    int N = a.size(), M = b.size();
    if (N == 0 || M == 0) {
        return N ? a : b;
    }
    vector<int> c(N + M - 1, INT_MAX);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            c[i + j] = min(c[i + j], a[i] + b[j]);
        }
    }
    return c;
}

auto naive_max_plus(const vector<int>& a, const vector<int>& b) {
    int N = a.size(), M = b.size();
    if (N == 0 || M == 0) {
        return N ? a : b;
    }
    vector<int> c(N + M - 1, INT_MIN);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            c[i + j] = max(c[i + j], a[i] + b[j]);
        }
    }
    return c;
}

auto random_sorted_vector(int N) {
    vector<int> A = rands_unif<int>(N, -50, 50);
    sort(begin(A), end(A));
    return A;
}

auto random_convex_vector(int N) {
    vector<int> A = rands_unif<int>(N, -50, 50);
    sort(begin(A), end(A));
    for (int i = 1; i < N; i++) {
        A[i] += A[i - 1];
    }
    return A;
}

auto random_concave_vector(int N) {
    vector<int> A = rands_unif<int>(N, -50, 50);
    sort(rbegin(A), rend(A));
    for (int i = 1; i < N; i++) {
        A[i] += A[i - 1];
    }
    return A;
}

void stress_test_min_plus_one_concave() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (5s, now, 100'000, runs) {
        print_time(now, 5s, "stress minplus concave check ({} runs)", runs);
        int N = rand_unif<int>(1, 100);
        int M = rand_unif<int>(1, 100);
        auto a = rands_unif<int>(N, -60, 60);
        auto b = random_concave_vector(M);
        auto c = min_plus_concave_one(a, b);
        auto d = naive_min_plus(a, b);
        assert(c == d);
    }
}

void stress_test_max_plus_one_convex() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (5s, now, 100'000, runs) {
        print_time(now, 5s, "stress maxplus convex check ({} runs)", runs);
        int N = rand_unif<int>(1, 100);
        int M = rand_unif<int>(1, 100);
        auto a = rands_unif<int>(N, -60, 60);
        auto b = random_convex_vector(M);
        auto c = max_plus_convex_one(a, b);
        auto d = naive_max_plus(a, b);
        assert(c == d);
    }
}

void stress_test_min_plus_concave_border() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (5s, now, 100'000, runs) {
        print_time(now, 5s, "stress minplus concave border ({} runs)", runs);
        int N = rand_unif<int>(0, 50);
        int M = rand_unif<int>(0, 50);
        auto a = random_concave_vector(N);
        auto b = random_concave_vector(M);
        auto c = min_plus_concave_border(a, b);
        auto d = naive_min_plus(a, b);
        assert(c == d);
    }
}

void stress_test_max_plus_convex_border() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (5s, now, 100'000, runs) {
        print_time(now, 5s, "stress minplus convex border ({} runs)", runs);
        int N = rand_unif<int>(0, 50);
        int M = rand_unif<int>(0, 50);
        auto a = random_convex_vector(N);
        auto b = random_convex_vector(M);
        auto c = max_plus_convex_border(a, b);
        auto d = naive_max_plus(a, b);
        assert(c == d);
    }
}

void stress_test_min_plus_smawk() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (5s, now, 100'000, runs) {
        print_time(now, 5s, "stress smawk ({} runs)", runs);
        int N = rand_unif<int>(1, 125);
        int M = rand_unif<int>(1, 125);
        auto a = rands_unif<int>(N, -1000, 1000);
        auto b = random_convex_vector(M);
        auto c = min_plus_smawk(a, b);
        auto d = naive_min_plus(a, b);
        assert(c == d);
    }
}

void stress_test_max_plus_smawk() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (5s, now, 100'000, runs) {
        print_time(now, 5s, "stress smawk ({} runs)", runs);
        int N = rand_unif<int>(1, 125);
        int M = rand_unif<int>(1, 125);
        auto a = rands_unif<int>(N, -1000, 1000);
        auto b = random_concave_vector(M);
        auto c = max_plus_smawk(a, b);
        auto d = naive_max_plus(a, b);
        assert(c == d);
    }
}

int main() {
    RUN_BLOCK(stress_test_min_plus_one_concave());
    RUN_BLOCK(stress_test_max_plus_one_convex());
    RUN_BLOCK(stress_test_min_plus_concave_border());
    RUN_BLOCK(stress_test_max_plus_convex_border());
    RUN_BLOCK(stress_test_min_plus_smawk());
    RUN_BLOCK(stress_test_max_plus_smawk());
    return 0;
}
