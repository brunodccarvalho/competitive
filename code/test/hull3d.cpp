#include "test_utils.hpp"
#include "geometry/generator3d.hpp"
#include "geometry/dachull.hpp"
#include "geometry/quickhull.hpp"

constexpr bool RUN_DACHULL = !Pt3::FLOAT, RUN_QUICKHULL = true;

void run_handmade_test(vector<Pt3>& pts, string file) {
    sort(begin(pts), end(pts));

    if constexpr (RUN_DACHULL) {
        auto hull = dachull::compute(pts);
        auto faces = Wedge::extract_faces(hull);
        Wedge::primary.release();
        write_obj_file(format("./geodebug/{}-dachull.obj", file), pts, faces);

        if (!check_hull(pts, faces)) {
            println("BUG DACHULL: {}", file);
            exit(1);
        }
    }

    if constexpr (RUN_QUICKHULL) {
        auto hull = quickhull::compute(pts);
        auto faces = Wedge::extract_faces(hull);
        Wedge::primary.release();
        write_obj_file(format("./geodebug/{}-quickhull.obj", file), pts, faces);

        if (!check_hull(pts, faces)) {
            println("BUG QUICKHULL: {}", file);
            exit(1);
        }
    }
}

void run_stress_test(vector<Pt3>& pts) {
    static int ERRORS = 0;

    if (RUN_DACHULL) {
        auto hull = dachull::compute(pts);
        auto faces = Wedge::extract_faces(hull);
        Wedge::primary.release();

        if (!check_hull(pts, faces)) {
            auto file = format("./geodebug/bug-{}-dachull.obj", ++ERRORS);
            write_obj_file(file, pts, faces);
            println("BUG DACHULL: {}", file);
        }
    }

    if (RUN_QUICKHULL) {
        auto hull = quickhull::compute(pts);
        auto faces = Wedge::extract_faces(hull);
        Wedge::primary.release();

        if (!check_hull(pts, faces)) {
            auto file = format("./geodebug/bug-{}-quickhull.obj", ++ERRORS);
            write_obj_file(file, pts, faces);
            println("BUG QUICKHULL: {}", file);
        }
    }

    if (ERRORS >= 30) {
        exit(1);
    }
}

void unit_test_hull3d() {
    vector<Pt3> pts;

    //  +--+-+   +-+    +--+  []  +--+-+---+-+----+--+  []
    pts = {
        Pt3(0, 0, 0),    Pt3(3, 3, 3),    Pt3(5, 5, 5),    Pt3(9, 9, 9),
        Pt3(11, 11, 11), Pt3(16, 16, 16), Pt3(19, 19, 19),
    };
    run_handmade_test(pts, "1d-line");

    //                        []                       []
    //         +              []      +_____________   []
    //        / \          +  []     /              +  []
    //       /   \        /|  []    /               |  []
    //      /     +      / |  []   /                |  []
    //     +     /      +  |  []  +                 |  []
    //      \   /        \ |  []   \                |  []
    //       \ /          \|  []    \               |  []
    //        +            +  []     +--------------+  []
    //                        []                       []
    pts = {
        Pt3(-10, -10, 0), Pt3(-15, -3, 0), Pt3(-8, 10, 0), Pt3(-4, 3, 0),
        Pt3(2, -4, 0),    Pt3(10, -10, 0), Pt3(10, 2, 0),
    };
    run_handmade_test(pts, "2d-regular");

    //        +        []        +        []
    //        |\       []       / \       []
    //      + | \      []      +   \      []
    //     /| |  \     []     /     \     []
    //    / | |   \    []    /       \    []
    //   /  | |    \   []   /         \   []
    //  +---+ +-----+  []  +---+-+-----+  []
    pts = {
        Pt3(0, 0, 0),  Pt3(5, 0, 0),   Pt3(5, 5, 0),
        Pt3(10, 0, 0), Pt3(10, 10, 0), Pt3(13, 0, 0),
    };
    run_handmade_test(pts, "2d-skip-triangle");

    //          +  []
    //          |  []  y
    //  +----+  |  []  |->x
    //          |  []
    //          +  []
    pts = {
        Pt3(0, 0, 0),
        Pt3(3, 0, 0),
        Pt3(5, -4, 0),
        Pt3(5, 4, 0),
    };
    run_handmade_test(pts, "2d-full-shadow-xy");

    rotate_coordinates_left(pts);
    run_handmade_test(pts, "2d-full-shadow-zx");

    rotate_coordinates_left(pts);
    run_handmade_test(pts, "2d-full-shadow-yz");

    //          +  []
    //          |  []  y
    //          |  []  |->x
    //          |  []
    //  +----+  +  []
    pts = {
        Pt3(0, 0, 0),
        Pt3(5, 0, 0),
        Pt3(10, 0, 0),
        Pt3(10, 5, 0),
    };
    run_handmade_test(pts, "2d-attach-xy");

    rotate_coordinates_left(pts);
    run_handmade_test(pts, "2d-attach-zx");

    rotate_coordinates_left(pts);
    run_handmade_test(pts, "2d-attach-yz");

    // https://imgur.com/a/Cy9cYZH but with upper 4 points collinear
    pts = {
        Pt3(10, 0, 0),  Pt3(5, 10, 0),  Pt3(10, 20, 0), Pt3(15, 21, 0),
        Pt3(20, 15, 0), Pt3(22, 8, 0),  Pt3(21, 0, 0),  Pt3(15, 0, 0),
        Pt3(30, 0, 0),  Pt3(35, 0, 0),  Pt3(36, 5, 0),  Pt3(40, 13, 0),
        Pt3(35, 25, 0), Pt3(30, 20, 0), Pt3(25, 23, 0), Pt3(24, 10, 0),
    };
    run_handmade_test(pts, "2d-hard-xy");

    rotate_coordinates_left(pts);
    run_handmade_test(pts, "2d-hard-zx");

    rotate_coordinates_left(pts);
    run_handmade_test(pts, "2d-hard-yz");

    // Non-degenerate tetrahedron
    pts = {
        Pt3(-5, -5, 0),
        Pt3(-5, 5, 0),
        Pt3(5, 0, -5),
        Pt3(5, 0, 5),
    };
    run_handmade_test(pts, "3d-tetrahedron");

    // Edge staring into triangle
    pts = {
        Pt3(0, 0, 0), Pt3(5, 0, 0), Pt3(10, 0, 5), Pt3(10, -5, -5), Pt3(10, 5, -5),
    };
    run_handmade_test(pts, "3d-stare-triangle");

    // Axis-aligned cube
    pts = {
        Pt3(-1, -1, -1), Pt3(-1, -1, 1), Pt3(-1, 1, -1), Pt3(-1, 1, 1),
        Pt3(1, -1, -1),  Pt3(1, -1, 1),  Pt3(1, 1, -1),  Pt3(1, 1, 1),
    };
    run_handmade_test(pts, "3d-cube");

    // Two axis-aligned cubes
    pts = {
        Pt3(-1, -1, -1), Pt3(-1, -1, 1), Pt3(-1, 1, -1), Pt3(-1, 1, 1),
        Pt3(1, -1, -1),  Pt3(1, -1, 1),  Pt3(1, 1, -1),  Pt3(1, 1, 1), //
        Pt3(3, 3, 3),    Pt3(3, 3, 5),   Pt3(3, 5, 3),   Pt3(3, 5, 5),
        Pt3(5, 3, 3),    Pt3(5, 3, 5),   Pt3(5, 5, 3),   Pt3(5, 5, 5),
    };
    run_handmade_test(pts, "3d-two-cubes");

    // Cube, but each face has another aligned square floating over and close to it
    pts = hypercube32();
    run_handmade_test(pts, "3d-hypercube32");

    //               +   []
    //              /|   []
    //             / |   []
    //    ,---+   +  |   []
    //  +----+|   |  +   []  |->x
    //    '---+   | /    []
    //            |/     []
    //            +      []
    pts = {
        Pt3(0, 0, 0),    Pt3(5, 1, 1),     Pt3(5, -1, 1),    Pt3(5, 0, -2),
        Pt3(15, 10, 10), Pt3(15, 10, -10), Pt3(15, -10, 10), Pt3(15, -10, -10),
    };
    run_handmade_test(pts, "3d-full-shadow-x");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-full-shadow-y");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-full-shadow-z");

    //               A   []
    //              /|   []
    //             / |   []
    //    ,---+   A  |   []  y
    //   +---+|   |  A   []  |->x   but all align perfectly
    //    \  |+   | /    []
    //     `-+    |/     []
    //            A      []
    pts = {
        Pt3(0, 0, 0),  Pt3(5, 1, 1),   Pt3(5, -1, 1),  Pt3(5, -1, -1),  Pt3(5, 1, -1),
        Pt3(15, 3, 3), Pt3(15, 3, -3), Pt3(15, -3, 3), Pt3(15, -3, -3),
    };
    run_handmade_test(pts, "3d-align-shadow-x");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-align-shadow-y");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-align-shadow-z");

    //               A   []
    //              /|   []
    //             / |   []
    //    ,---+   S  |   []  y
    //   +---+|   |  S   []  |->x   two opposite align perfectly
    //    \  |+   | /    []
    //     `-+    |/     []
    //            A      []
    pts = {
        Pt3(0, 0, 0),  Pt3(5, 1, 1),   Pt3(5, -1, 1),  Pt3(5, -1, -1),  Pt3(5, 1, -1),
        Pt3(15, 5, 5), Pt3(15, 3, -3), Pt3(15, -3, 3), Pt3(15, -5, -5),
    };
    run_handmade_test(pts, "3d-opposite-shadow-x");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-opposite-shadow-y");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-opposite-shadow-z");

    //               S   []
    //              /|   []
    //             / |   []
    //    ,---+   S  |   []
    //   +---+|   |  S   []  |->x   one aligns perfectly
    //    \  |+   | /    []
    //     `-+    |/     []
    //            A      []
    pts = {
        Pt3(0, 0, 0),  Pt3(5, 1, 1),   Pt3(5, -1, 1),  Pt3(5, -1, -1),  Pt3(5, 1, -1),
        Pt3(15, 3, 3), Pt3(15, 3, -3), Pt3(15, -3, 3), Pt3(15, -5, -5),
    };
    run_handmade_test(pts, "3d-one-shadow-x");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-one-shadow-y");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-one-shadow-z");

    //               A   []
    //              /|   []
    //             / |   []
    //    ,---+   S  |   []
    //   +    |   |  S   []  |->x   corners align
    //    `---+   | /    []
    //            |/     []
    //            A      []
    pts = {
        Pt3(0, 0, 0),   Pt3(5, 1, 1),   Pt3(5, -1, -1),  Pt3(15, 3, 3),
        Pt3(15, 3, -3), Pt3(15, -3, 3), Pt3(15, -3, -3),
    };
    run_handmade_test(pts, "3d-align-shadow-tri-x");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-align-shadow-tri-y");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-align-shadow-tri-z");

    //               S   []
    //              /|   []
    //             / |   []
    //    ,---+   S  |   []
    //   +    |   |  S   []  |->x   one corner aligns
    //    `---+   | /    []
    //            |/     []
    //            A      []
    pts = {
        Pt3(0, 0, 0),   Pt3(5, 1, 1),   Pt3(5, -1, -1),  Pt3(15, 3, 3),
        Pt3(15, 3, -3), Pt3(15, -3, 3), Pt3(15, -5, -5),
    };
    run_handmade_test(pts, "3d-one-shadow-tri-x");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-one-shadow-tri-y");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-one-shadow-tri-z");

    //               S   []
    //              /|   []
    //             / |   []
    //    ,---+   S  |   []
    //   +    |   |  S   []  |->x   all corners shadow
    //    `---+   | /    []
    //            |/     []
    //            S      []
    pts = {
        Pt3(0, 0, 0),   Pt3(5, 1, 1),   Pt3(5, -1, -1),  Pt3(15, 5, 5),
        Pt3(15, 3, -3), Pt3(15, -3, 3), Pt3(15, -5, -5),
    };
    run_handmade_test(pts, "3d-full-shadow-tri-x");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-full-shadow-tri-y");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-full-shadow-tri-z");

    // Two square hourglass pyramids with bases facing away from each other
    pts = {
        Pt3(-2, 0, 0),    Pt3(-10, -10, -10), Pt3(-10, -10, 10), Pt3(-10, 10, -10),
        Pt3(-10, 10, 10), Pt3(2, 0, 0),       Pt3(10, -10, -10), Pt3(10, -10, 10),
        Pt3(10, 10, -10), Pt3(10, 10, 10),
    };
    run_handmade_test(pts, "3d-square-hourglass-x");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-square-hourglass-y");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-square-hourglass-z");

    // Similar, small hourglass has large number of vertices, large small number
    pts = {
        Pt3(-10, 0, 0),   Pt3(0, 4, 0),  Pt3(0, 3, 2),  Pt3(0, 2, 3),   Pt3(0, 0, 4),
        Pt3(0, -2, 3),    Pt3(0, -3, 2), Pt3(0, -4, 0), Pt3(0, -3, -2), Pt3(0, -2, -3),
        Pt3(0, 0, -4),    Pt3(0, 2, -3), Pt3(0, 3, -2), Pt3(5, 0, 25),  Pt3(5, 20, -10),
        Pt3(5, -20, -10), Pt3(10, 0, 0), Pt3(11, 0, 0), Pt3(12, 0, 0),  Pt3(13, 0, 0),
        Pt3(14, 0, 0),    Pt3(15, 0, 0), Pt3(16, 0, 0), Pt3(17, 0, 0),  Pt3(18, 0, 0),
    };
    run_handmade_test(pts, "3d-uneven-hourglass-x");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-uneven-hourglass-y");

    rotate_coordinates_right(pts);
    run_handmade_test(pts, "3d-uneven-hourglass-z");
}

void stress_test_hull3d() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (90s, now, 50000, runs) {
        int N = rand_unif<int>(2, 800);
        int L = cointoss(0.5) ? rand_wide<int>(0, N, -1) : 0;
        int C = cointoss(0.5) ? rand_wide<int>(0, N, -1) : 0;
        int K = N + L + C;
        PointDistrib distr = rand_point_distribution();

        print_time(now, 90s, "stress hull3d {} runs {:4} {}", runs, K, to_string(distr));

        vector<Pt3> pts = generate_points(N, distr, L, C, 100'000'000);
        run_stress_test(pts);
    }
}

void speed_test_hull3d() {
    vector<int> Ns = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};

    vector<tuple<int, PointDistrib>> inputs;
    for (int N : Ns) {
        for (int distr = 0; distr < int(PointDistrib::END); distr = distr + 1) {
            inputs.emplace_back(N, PointDistrib(distr));
        }
    }

    auto runtime = 480'000ms / inputs.size();
    map<tuple<stringable, int, string>, stringable> table;

    for (auto [N, distr] : inputs) {
        START_ACC2(dac, quick);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 1000, runs) {
            print_time(now, runtime, "speed 3d hull  {} N={}", to_string(distr), N);

            auto pts = generate_points(N, distr, 0, 0, 100'000'000);

            if constexpr (RUN_DACHULL) {
                ADD_TIME_BLOCK(dac) {
                    dachull::compute(pts);
                    Wedge::primary.release();
                }
            }
            if constexpr (RUN_QUICKHULL) {
                ADD_TIME_BLOCK(quick) {
                    quickhull::compute(pts);
                    Wedge::primary.release();
                }
            }
        }

        if constexpr (RUN_DACHULL) {
            table[{distr, N, "d&c"}] = FORMAT_EACH(dac, runs);
        }
        if constexpr (RUN_QUICKHULL) {
            table[{distr, N, "quick"}] = FORMAT_EACH(quick, runs);
        }
    }

    print_time_table(table, "3d Hull");
}

int main() {
    RUN_BLOCK(unit_test_hull3d());
    RUN_BLOCK(stress_test_hull3d());
    RUN_BLOCK(speed_test_hull3d());
    return 0;
}
