#include "test_utils.hpp"
#include "geometry/geometry3d.hpp"
#include "geometry/double3d.hpp"
#include "geometry/generator3d.hpp"

void stress_test_sorters() {
    vector<Pt3> pts;
    for (int x = -9; x <= 9; x++) {
        for (int y = -9; y <= 9; y++) {
            for (int z = -9; z <= 9; z++) {
                pts.emplace_back(x, y, z);
            }
        }
    }
    int N = pts.size();
    debug(N);

    {
        sort(begin(pts), end(pts),
             [](auto u, auto v) { return azimuth_angle_sort(u, v); });
        auto A = pts;

        sort(begin(pts), end(pts),
             [](auto u, auto v) { return azimuth_sweep_sort(u, v); });
        auto B = pts;

        sort(begin(pts), end(pts),
             [](auto u, auto v) { return angle_azimuth_sort(u, v); });
        auto C = pts;

        sort(begin(pts), end(pts),
             [](auto u, auto v) { return sweep_azimuth_sort(u, v); });
        auto D = pts;

        // Verify there is full tie breaking
        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                assert(azimuth_angle_sort(A[i], A[j]));
                assert(azimuth_sweep_sort(B[i], B[j]));
                assert(angle_azimuth_sort(C[i], C[j]));
                assert(sweep_azimuth_sort(D[i], D[j]));
                assert(!azimuth_angle_sort(A[j], A[i]));
                assert(!azimuth_sweep_sort(B[j], B[i]));
                assert(!angle_azimuth_sort(C[j], C[i]));
                assert(!sweep_azimuth_sort(D[j], D[i]));
            }
            assert(!azimuth_angle_sort(A[i], A[i]));
            assert(!azimuth_sweep_sort(B[i], B[i]));
            assert(!angle_azimuth_sort(C[i], C[i]));
            assert(!sweep_azimuth_sort(D[i], D[i]));
        }
    }

    LOOP_FOR_DURATION_OR_RUNS_TRACKED (30s, now, 1000, runs) {
        print_time(now, 30s, "stress 3d sorters ({} runs)", runs);

        auto pivot = rand_ball(10000);
        auto axis = rand_sphere(10000);
        auto forward = rand_sphere(10000);

        if (samedir(axis, forward)) {
            continue;
        }

        auto A_sorter = azimuth_angle_sorter(pivot, axis, forward);
        auto B_sorter = azimuth_sweep_sorter(pivot, axis, forward);
        auto C_sorter = angle_azimuth_sorter(pivot, axis, forward);
        auto D_sorter = sweep_azimuth_sorter(pivot, axis, forward);

        sort(begin(pts), end(pts), A_sorter);
        auto A = pts;

        sort(begin(pts), end(pts), B_sorter);
        auto B = pts;

        sort(begin(pts), end(pts), C_sorter);
        auto C = pts;

        sort(begin(pts), end(pts), D_sorter);
        auto D = pts;

        // Verify there is full tie breaking
        for (int i = 0; i < N; i++) {
            for (int j = i + 1; j < N; j++) {
                assert(A_sorter(A[i], A[j]));
                assert(B_sorter(B[i], B[j]));
                assert(C_sorter(C[i], C[j]));
                assert(D_sorter(D[i], D[j]));
                assert(!A_sorter(A[j], A[i]));
                assert(!B_sorter(B[j], B[i]));
                assert(!C_sorter(C[j], C[i]));
                assert(!D_sorter(D[j], D[i]));
            }
            assert(!A_sorter(A[i], A[i]));
            assert(!B_sorter(B[i], B[i]));
            assert(!C_sorter(C[i], C[i]));
            assert(!D_sorter(D[i], D[i]));
        }
    }
}

int main() {
    RUN_BLOCK(stress_test_sorters());
    return 0;
}
