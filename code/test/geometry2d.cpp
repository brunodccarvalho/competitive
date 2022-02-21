#include "test_utils.hpp"
#include "geometry/geometry2d.hpp"
#include "geometry/cuts.hpp"
#include "geometry/double2d.hpp"
#include "geometry/rotating_calipers.hpp"
#include "geometry/all_point_pairs.hpp"
#include "geometry/hull2d.hpp"
#include "geometry/point_location.hpp"
#include "geometry/utils2d.hpp"
#include "geometry/generator2d.hpp"

void unit_test_offline_point_location() {
    mt.seed(73);
    int N = 60, S = 30;

    auto pts = generate_points(N / 4, PointDistrib::SQUARE, 0, 12);
    auto segments = non_overlapping_sample(pts, S, {});
    S = segments.size();

    auto more = generate_points(N - N / 4, PointDistrib::SQUARE, 0, 12);
    pts.insert(end(pts), begin(more), end(more));

    auto [hit_ignore_below, hit_ignore_above] = offline_point_location(pts, segments,
                                                                       HIT_IGNORE);
    auto [hit_above_below, hit_above_above] = offline_point_location(pts, segments,
                                                                     HIT_ABOVE);
    auto [hit_below_below, hit_below_above] = offline_point_location(pts, segments,
                                                                     HIT_BELOW);

    write_obj_file("./geodebug/location.obj", pts, segments);
    debug(segments);
    debug(pts[0]);

    vector<string> names(S + 1);
    for (int i = 0; i < S; i++) {
        names[i + 1] = format("[{:2}-{:2}]", segments[i][0], segments[i][1]);
    }

    for (int i = 0; i < N; i++) {
        string a = names[1 + hit_ignore_below[i]];
        string b = names[1 + hit_ignore_above[i]];
        string c = names[1 + hit_above_below[i]];
        string d = names[1 + hit_above_above[i]];
        string e = names[1 + hit_below_below[i]];
        string f = names[1 + hit_below_above[i]];

        println("[{:2}]: ignore({:>7} {:>7}) above({:>7} {:>7}) below({:>7} {:>7})", i, a,
                b, c, d, e, f);
    }
}

void stress_test_hull2d() {
    LOOP_FOR_DURATION_TRACKED_RUNS (20s, now, runs) {
        print_time(now, 20s, "stress hull 2d ({} runs)", runs);

        int N = rand_unif<int>(1, 200);
        int L = cointoss(0.5) && N > 1 ? rand_unif<int>(1, N) : 0;
        auto distr = rand_point_distribution();

        auto pts = generate_points(N, distr, L, 800);
        auto more = generate_points(N, distr, 0, 800);
        pts.insert(end(pts), begin(more), end(more));

        auto index_hull1 = hull_monotone_chain(pts);
        auto index_hull2 = hull_divide_conquer(pts);
        auto hull1 = extract_points(pts, index_hull1);
        auto hull2 = extract_points(pts, index_hull2);
        auto hull3 = hull_divide_conquer_inplace(pts);

        check_hull_exact(pts, hull1, true);
        check_hull_exact(pts, hull2, true);
        check_hull_exact(pts, hull3, true);
    }
}

void speed_test_hull2d() {
    vector<int> Ns = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};

    vector<tuple<int, PointDistrib>> inputs;
    for (int N : Ns) {
        for (int distr = 0; distr < int(PointDistrib::END); distr = distr + 1) {
            inputs.emplace_back(N, PointDistrib(distr));
        }
    }

    auto runtime = 240'000ms / inputs.size();
    map<pair<stringable, int>, stringable> table;

    for (auto [N, distr] : inputs) {
        START_ACC(hull2d);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 1000, runs) {
            print_time(now, runtime, "speed hull2d {} N={}", to_string(distr), N);

            auto pts = generate_points(N, distr, 0, 100'000'000);

            ADD_TIME_BLOCK(hull2d) { hull_monotone_chain(pts); }
        }

        table[{distr, N}] = FORMAT_EACH(hull2d, runs);
    }

    print_time_table(table, "Hull 2d");
}

void stress_test_merge_hulls() {
    LOOP_FOR_DURATION_TRACKED_RUNS (20s, now, runs) {
        print_time(now, 20s, "stress merge hull 2d ({} runs)", runs);

        int A = rand_unif<int>(1, 200);
        int B = rand_unif<int>(1, 200);
        int La = cointoss(0.5) && A > 1 ? rand_unif<int>(1, A) : 0;
        int Lb = cointoss(0.5) && B > 1 ? rand_unif<int>(1, B) : 0;

        auto apts = generate_points(A, rand_point_distribution(), La, 800);
        auto bpts = generate_points(B, rand_point_distribution(), Lb, 800);
        A = apts.size(), B = bpts.size();

        auto pts = apts;
        pts.insert(end(pts), begin(bpts), end(bpts));

        auto ci = hull_monotone_chain(apts);
        auto di = hull_monotone_chain(bpts);
        auto c = extract_points(apts, ci);
        auto d = extract_points(bpts, di);
        auto hull1 = merge_hulls(c, d);

        check_hull_exact(apts, hull1);
        check_hull_exact(bpts, hull1);
        check_hull_exact(c, hull1);
        check_hull_exact(d, hull1);
        check_hull_exact(pts, hull1, true);

        for (int& i : di) {
            i += A;
        }
        auto index_hull2 = merge_hulls(pts, ci, di);
        auto hull2 = extract_points(pts, index_hull2);

        check_hull_exact(apts, hull2);
        check_hull_exact(bpts, hull2);
        check_hull_exact(c, hull2);
        check_hull_exact(d, hull2);
        check_hull_exact(pts, hull2, true);
    }
}

void stress_test_all_point_pairs_radial_sweep() {
    LOOP_FOR_DURATION_TRACKED_RUNS (20s, now, runs) {
        print_time(now, 20s, "stress all point pairs 2d ({} runs)", runs);

        int N = rand_unif<int>(1, 200);
        int L = cointoss(0.5) && N > 1 ? rand_unif<int>(1, N) : 0;

        auto pts = generate_points(N, rand_point_distribution(), L, 800);

        all_point_pairs_radial_sweep(pts, [&](const auto& index, int x, int u, int v) {
            assert(u == index[x] && v == index[x + 1]);

            // verify the points are indeed sorted by their signed distance to line uv
            for (int i = 1; i < N; i++) {
                assert(signed_linedist(pts[index[i]], pts[u], pts[v]) >=
                       signed_linedist(pts[index[i - 1]], pts[u], pts[v]));
            }
        });
    }
}

void stress_test_separating_line() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (10s, now, 100'000, runs) {
        print_time(now, 10s, "stress separating line ({} runs)", runs);

        int N = rand_unif<int>(2, 100);
        int L = cointoss(0.5) ? rand_unif<int>(1, N) : 0;
        auto distr = rand_point_distribution();

        auto pts = generate_points(N, distr, L, 10'000);
        auto dir = rand_circle(50'000'000);

        int R = rand_unif<int>(1, N - 1);
        nth_element(begin(pts), begin(pts) + R, end(pts), line_sorter(dir));

        shuffle(begin(pts), begin(pts) + R, mt);
        shuffle(begin(pts) + R, end(pts), mt);
        vector<Pt2> reds(begin(pts), begin(pts) + R);
        vector<Pt2> blues(begin(pts) + R, end(pts));

        auto [ok1, r1, b1] = separating_line(reds, blues, +1);
        auto [ok2, r2, b2] = separating_line(reds, blues, -1);
        auto l1 = Ray::through(reds[r1], blues[b1]);
        auto l2 = Ray::through(reds[r2], blues[b2]);

        assert(ok1 && ok2);
        check_separating_line(reds, blues, l1, +1);
        check_separating_line(reds, blues, l2, -1);
    }
}

void speed_test_separating_line() {
    vector<int> Ns = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};

    vector<tuple<int, PointDistrib>> inputs;
    for (int N : Ns) {
        for (int distr = 0; distr < int(PointDistrib::END); distr = distr + 1) {
            inputs.emplace_back(N, PointDistrib(distr));
        }
    }

    auto runtime = 90'000ms / inputs.size();
    map<pair<stringable, int>, stringable> table;

    for (auto [N, distr] : inputs) {
        START_ACC(sep);

        const int INSIDE = 10;

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 1000, runs) {
            print_time(now, runtime, "speed sep. line {} N={}", to_string(distr), N);

            auto pts = generate_points(N, distr, 0, 100'000'000);

            for (int run = 0; run < INSIDE; run++) {
                auto dir = rand_circle(10000);
                int R = rand_unif<int>(1, N - 1);
                nth_element(begin(pts), begin(pts) + R, end(pts), line_sorter(dir));

                shuffle(begin(pts), begin(pts) + R, mt);
                shuffle(begin(pts) + R, end(pts), mt);
                vector<Pt2> reds(begin(pts), begin(pts) + R);
                vector<Pt2> blues(begin(pts) + R, end(pts));

                ADD_TIME_BLOCK(sep) {
                    auto [ok1, r1, b1] = separating_line(reds, blues, +1);
                    auto [ok2, r2, b2] = separating_line(reds, blues, -1);
                    assert(ok1 && ok2);
                }
            }
        }

        table[{distr, N}] = FORMAT_EACH(sep, 2 * INSIDE * runs);
    }

    print_time_table(table, "Separating line");
}

void stress_test_sandwich_line() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (30s, now, 100'000, runs) {
        print_time(now, 30s, "stress sandwich line ({} runs)", runs);

        int R = rand_unif<int>(1, 500);
        int B = rand_unif<int>(1, 500);
        auto distr = rand_point_distribution();

        auto pts = generate_points(R + B, distr, 0, 500'000);
        vector<Pt2> reds(begin(pts), begin(pts) + R);
        vector<Pt2> blues(begin(pts) + R, end(pts));

        auto [r, b] = sandwich_line(reds, blues);
        auto line = Ray::through(reds[r], blues[b]);
        assert(check_sandwich_line(reds, blues, line));
    }
}

void speed_test_sandwich_line() {
    vector<int> Ns = {/*6000, 15000, 30000, 50000, 100000, 200000, 300000,*/ 500000,
                      800000};

    vector<tuple<int, PointDistrib>> inputs;
    for (int N : Ns) {
        for (int d = 0; d < int(PointDistrib::END); d = d + 1) {
            inputs.emplace_back(N, PointDistrib(d));
        }
    }
    auto runtime = 90'000ms / inputs.size();
    map<pair<stringable, int>, stringable> table;

    for (auto [N, distr] : inputs) {
        START_ACC(sand);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 1000, runs) {
            print_time(now, runtime, "speed sandwich line {} N={}", to_string(distr), N);

            auto pts = generate_points(N, distr, 0, 300'000'000);
            vector<Pt2> reds(begin(pts), begin(pts) + N / 2);
            vector<Pt2> blues(begin(pts) + N / 2, end(pts));

            ADD_TIME_BLOCK(sand) { sandwich_line(reds, blues); }
        }

        table[{distr, N}] = FORMAT_EACH(sand, runs);
    }

    print_time_table(table, "Sandwich line");
}

void stress_test_approximate_sandwich_line() {
    LOOP_FOR_DURATION_OR_RUNS_TRACKED (30s, now, 100'000, runs) {
        print_time(now, 30s, "stress approximate sandwich line ({} runs)", runs);

        int N = rand_unif<int>(1, 500);

        auto pts = generate_points(2 * N, PointDistrib::QUAD_MODULO, 0, 500'000);
        vector<Pt2> reds(begin(pts), begin(pts) + N);
        vector<Pt2> blues(begin(pts) + N, end(pts));

        auto [r, b] = approximate_sandwich_line(reds, blues);
        auto line = Ray::through(reds[r], blues[b]);
        assert(check_approximate_sandwich_line(reds, blues, line));
    }
}

void speed_test_approximate_sandwich_line() {
    vector<int> Ns = {6000, 15000, 30000, 50000, 100000, 200000, 300000, 500000, 800000};

    vector<tuple<int, PointDistrib>> inputs;
    for (int N : Ns) {
        inputs.emplace_back(N, PointDistrib::QUAD_MODULO);
    }

    auto runtime = 120'000ms / inputs.size();
    map<pair<stringable, int>, stringable> table;

    for (auto [N, distr] : inputs) {
        START_ACC(sandw);

        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 5000, runs) {
            print_time(now, runtime, "speed approximate sandwich {} N={}",
                       to_string(distr), N);

            auto pts = generate_points(N, distr, 0, 300'000'000);
            vector<Pt2> reds(begin(pts), begin(pts) + N / 2);
            vector<Pt2> blues(begin(pts) + N / 2, end(pts));

            ADD_TIME_BLOCK(sandw) { approximate_sandwich_line(reds, blues); }
        }

        table[{distr, N}] = FORMAT_EACH(sandw, runs);
    }

    print_time_table(table, "Approximate sandwich line");
}

int main() {
    RUN_BLOCK(unit_test_offline_point_location());
    return 0;
    // RUN_BLOCK(stress_test_hull2d());
    // RUN_BLOCK(stress_test_merge_hulls());
    // RUN_BLOCK(stress_test_all_point_pairs_radial_sweep());
    // RUN_BLOCK(stress_test_separating_line());
    // RUN_BLOCK(speed_test_separating_line());
    // RUN_BLOCK(stress_test_sandwich_line());
    RUN_BLOCK(speed_test_sandwich_line());
    // RUN_BLOCK(stress_test_approximate_sandwich_line());
    RUN_BLOCK(speed_test_approximate_sandwich_line());
    RUN_BLOCK(speed_test_hull2d());
    return 0;
}
