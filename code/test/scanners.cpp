#include "test_utils.hpp"
#include "geometry/shaft_scanner.hpp"
#include "geometry/generator2d.hpp"
#include "geometry/utils2d.hpp"

bool operator<(const Ray& u, const Ray& v) {
    return make_pair(u.p, u.q()) < make_pair(v.p, v.q());
}

void stress_test_line_scanner() {
    LOOP_FOR_DURATION_TRACKED_RUNS (30s, now, runs) {
        print_time(now, 30s, "stress sweep scanner ({} runs)", runs);

        int N = rand_wide<int>(3, 100, -1);
        int S = rand_unif<int>(3, min(150, 2 * N));
        auto distr = rand_point_distribution();
        auto forward = rand_circle(20);

        auto pts = generate_points(N, distr, 0, 200);
        auto segments = non_overlapping_sample(pts, S, {}, true);

        N = pts.size(), S = segments.size();
        auto sorter = line_sorter(forward);
        auto scanner = live_sweep_scanner(forward);

        // Sort points and orient segments
        vector<int> index(N);
        iota(begin(index), end(index), 0);
        sort(begin(index), end(index),
             [&](int i, int j) { return sorter(pts[i], pts[j]); });

        vector<int> rank(N);
        for (int i = 0; i < N; i++) {
            rank[index[i]] = i;
        }

        vector<int> head(N, -1), head_next(S, -1);
        vector<int> tail(N, -1), tail_next(S, -1);

        for (int i = 0; i < S; i++) {
            auto& [u, v] = segments[i];
            assert(u != v);
            if (!scanner.oriented(Ray::through(pts[u], pts[v]))) {
                assert(rank[u] > rank[v]);
                swap(u, v);
                assert(scanner.oriented(Ray::through(pts[u], pts[v])));
            }
            head_next[i] = head[u], head[u] = i;
            tail_next[i] = tail[v], tail[v] = i;
        }

        vector<Ray> lines(S);
        for (int i = 0; i < S; i++) {
            auto [u, v] = segments[i];
            lines[i] = Ray::through(pts[u], pts[v]);
        }

        // Populate scanset in order and verify insertions and deletions
        set<Ray, live_sweep_scanner> scanset(scanner);

        for (int loop = 0; loop < 4; loop++) {
            for (int u : index) {
                for (int i = tail[u]; i != -1; i = tail_next[i]) {
                    auto erased = scanset.erase(lines[i]);
                    assert(erased == 1);
                }
                for (int i = head[u]; i != -1; i = head_next[i]) {
                    bool inserted = scanset.insert(lines[i]).second;
                    assert(inserted);
                }
            }
            assert(scanset.empty());
        }

        // Populate scanmap in order and verify stab predicate
        map<Ray, int, live_sweep_scanner> scanmap(scanner);
        vector<vector<int8_t>> simultaneous(S, vector<int8_t>(S));

        for (int u : index) {
            for (int i = tail[u]; i != -1; i = tail_next[i]) {
                scanmap.erase(lines[i]);
            }
            for (int i = head[u]; i != -1; i = head_next[i]) {
                scanmap.emplace(lines[i], i);
            }
            for (int i = head[u]; i != -1; i = head_next[i]) {
                for (const auto& [ray, a] : scanmap) {
                    simultaneous[i][a] = simultaneous[a][i] = true;
                }
            }
        }
    }
}

void stress_test_angle_scanner() {
    LOOP_FOR_DURATION_TRACKED_RUNS (30s, now, runs) {
        print_time(now, 30s, "stress angle scanner ({} runs)", runs);

        int N = rand_wide<int>(3, 100, -1);
        int S = rand_unif<int>(3, min(150, 2 * N));
        auto distr = rand_point_distribution();
        Pt2 pivot = rand_disk(20);

        auto pts = generate_points(N, distr, 0, 200);
        pts.erase(remove(begin(pts), end(pts), pivot), end(pts));
        auto segments = non_overlapping_sample(pts, S, {{pivot, pivot}}, true);

        N = pts.size(), S = segments.size();
        auto sorter = angle_sorter(pivot);
        auto scanner = live_angle_scanner(pivot);

        // Sort points, orient segments, and collect initial segments
        vector<int> index(N);
        iota(begin(index), end(index), 0);
        sort(begin(index), end(index),
             [&](int i, int j) { return sorter(pts[i], pts[j]); });

        vector<int> rank(N);
        for (int i = 0; i < N; i++) {
            rank[index[i]] = i;
        }

        vector<int> head(N, -1), head_next(S, -1);
        vector<int> tail(N, -1), tail_next(S, -1);
        vector<int> initial;

        for (int i = 0; i < S; i++) {
            auto& [u, v] = segments[i];
            assert(u != v);
            if (!scanner.oriented(Ray::through(pts[u], pts[v]))) {
                swap(u, v);
                assert(scanner.oriented(Ray::through(pts[u], pts[v])));
            }
            if (rank[u] > rank[v]) {
                initial.push_back(i);
            }
            head_next[i] = head[u], head[u] = i;
            tail_next[i] = tail[v], tail[v] = i;
        }

        vector<Ray> lines(S);
        for (int i = 0; i < S; i++) {
            auto [u, v] = segments[i];
            lines[i] = Ray::through(pts[u], pts[v]);
        }

        // Populate scanset in order and verify insertions and deletions
        set<Ray, live_angle_scanner> scanset(scanner);

        for (int i : initial) {
            scanset.insert(lines[i]);
        }

        for (int loop = 0; loop < 4; loop++) {
            for (int u : index) {
                for (int i = tail[u]; i != -1; i = tail_next[i]) {
                    auto erased = scanset.erase(lines[i]);
                    assert(erased == 1);
                }
                for (int i = head[u]; i != -1; i = head_next[i]) {
                    bool inserted = scanset.insert(lines[i]).second;
                    assert(inserted);
                }
            }
            assert(scanset.size() == initial.size());
        }

        // Populate scanmap in order and verify stab predicate
        map<Ray, int, live_angle_scanner> scanmap(scanner);
        vector<vector<int8_t>> simultaneous(S, vector<int8_t>(S));

        for (int i : initial) {
            scanmap.emplace(lines[i], i);
        }
        for (int i : initial) {
            for (int j : initial) {
                simultaneous[i][j] = simultaneous[j][i] = true;
            }
        }

        for (int u : index) {
            for (int i = tail[u]; i != -1; i = tail_next[i]) {
                scanmap.erase(lines[i]);
            }
            for (int i = head[u]; i != -1; i = head_next[i]) {
                scanmap.emplace(lines[i], i);
            }
            for (int i = head[u]; i != -1; i = head_next[i]) {
                for (const auto& [ray, a] : scanmap) {
                    simultaneous[i][a] = simultaneous[a][i] = true;
                }
            }
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_line_scanner());
    RUN_BLOCK(stress_test_angle_scanner());
    return 0;
}
