#include "test_utils.hpp"
#include "geometry/voronoi.hpp"
#include "geometry/generator2d.hpp"

void speed_test_voronoi() {
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
        START_ACC(voronoi);

        LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
            print_time(now, runtime, "speed voronoi {} N={}", N, to_string(dist));

            auto pts = generate_points(N, dist, 0, 10'000'000);

            ADD_TIME_BLOCK(voronoi) {
                auto [data, centers] = voronoi(pts);
                box_voronoi(pts, data[0], centers, VoronoiBox{});
                Wedge::release();
            }
        }

        table[{dist, N}] = FORMAT_EACH(voronoi, runs);
    }

    print_time_table(table, "Voronoi Diagram");
}

int main() {
    RUN_BLOCK(speed_test_voronoi());
    return 0;
}
