#include "test_utils.hpp"
#include "geometry/delaunay.hpp"
#include "geometry/generator2d.hpp"

void stress_test_delaunay() {
    LOOP_FOR_DURATION_TRACKED_RUNS (20s, now, runs) {
        print_time(now, 20s, "stress delaunay ({} runs)", runs);

        // Degenerate distributions too hard for floating points :(
        int N = rand_unif<int>(4, 300);
        auto distr = rand_point_distribution();
        auto pts = generate_points(N, distr, 0);

        auto data = delaunay(pts);
        auto faces = Wedge::extract_faces(data);
        vector<array<int, 3>> triangles;
        check_constrained_triangulation(faces, pts, {});

        for (int i = 1, F = faces.size(); i < F; i++) {
            assert(faces[i].size() == 3u);
            triangles.push_back({faces[i][0], faces[i][1], faces[i][2]});
        }

        for (auto [a, b, c] : triangles) {
            for (int i = 0; i < N; i++) {
                assert(!inside_circumference(pts[i], pts[a], pts[b], pts[c]));
            }
            assert(orientation(pts[a], pts[b], pts[c]) > 0);
        }
        Wedge::release();
    }
}

void speed_test_delaunay() {
    vector<int> Ns = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};

    vector<pair<int, PointDistrib>> inputs;
    for (int N : Ns) {
        for (int i = 0; i < int(PointDistrib::END); i++) {
            inputs.push_back({N, PointDistrib(i)});
        }
    }

    const auto runtime = 180000ms / inputs.size();
    map<pair<stringable, int>, string> table;

    for (auto [N, dist] : inputs) {
        START_ACC(delaunay);

        LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
            print_time(now, runtime, "speed delaunay {} N={}", N, to_string(dist));

            auto pts = generate_points(N, dist, 0, 10'000'000);

            ADD_TIME_BLOCK(delaunay) {
                delaunay(pts);
                Wedge::release();
            }
        }

        table[{dist, N}] = FORMAT_EACH(delaunay, runs);
    }

    print_time_table(table, "Delaunay Triangulation");
}

int main() {
    RUN_BLOCK(stress_test_delaunay());
    RUN_BLOCK(speed_test_delaunay());
    return 0;
}
