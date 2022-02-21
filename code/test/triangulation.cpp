#include "test_utils.hpp"
#include "geometry/triangulation.hpp"
#include "geometry/utils2d.hpp"
#include "geometry/generator2d.hpp"

void stress_test_constrained_triangulation() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (30s, now, 20'000, runs) {
        print_time(now, 30s, "stress constrained triangulation {} runs", runs);

        int N = rand_unif<int>(5, 150);
        int S = rand_unif<int>(5, min(70, 2 * N));
        auto dist = rand_point_distribution();

        auto pts = generate_points(N, dist, 0);
        auto segments = non_overlapping_sample(pts, S, {}, false);

        Wedge* edge = constrained_triangulation(pts, segments);
        auto faces = Wedge::extract_faces(edge);

        assert(check_constrained_triangulation(faces, pts, segments));
        Wedge::release();
    }
}

int main() {
    RUN_BLOCK(stress_test_constrained_triangulation());
    return 0;
}
