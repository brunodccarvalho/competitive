#include "test_utils.hpp"
#include "hash.hpp"
#include "numeric/math.hpp"
#include "linear/matrix.hpp"
#include "numeric/partitions.hpp"

template <typename T>
auto fmthist(const map<T, int>& cnt) {
    const int W = 150;
    int64_t M = 0, C = 0, m = INT64_MAX;
    vector<int> cnts;
    for (auto [n, c] : cnt) {
        M = max<int64_t>(M, c);
        m = min<int64_t>(m, c);
        C += c;
        cnts.push_back(c);
    }
    int S = cnts.size();
    string lead;
    lead += format("C={}, M/m={:f}\n", C, 1.0 * M / m);
    for (int i = 0; i + 1 < S && S <= 50; i++) {
        lead += format("{:>4.3f}{}", 1.0 * cnts[i] / cnts[i + 1], " \n"[i + 2 == S]);
    }
    for (auto [n, c] : cnt) {
        int w = (1LL * c * W + M / 2) / M;
        double p = 100.0 * c / C;
        lead += format("{:>18} {:>6.3f}% {}\n", n, p, string(w, '#'));
    }
    return lead;
}

template <typename T>
auto fmthistweighted(const map<T, int>& cnt) {
    int W = 150;
    int64_t C = 0, M = 0, m = INT64_MAX;
    vector<int64_t> cnts;
    for (auto [n, c] : cnt) {
        M = max<int64_t>(M, 1LL * c * n);
        m = min<int64_t>(m, 1LL * c * n);
        C += 1LL * c * n;
        cnts.push_back(1LL * c * n);
    }
    int S = cnts.size();
    string lead;
    lead += format("M/m: {:7.4f}\n", 1.0 * M / m);
    for (int i = 0; i + 1 < S && S <= 40; i++) {
        lead += format("{:>4.3f}{}", 1.0 * cnts[i] / cnts[i + 1], " \n"[i + 2 == S]);
    }
    for (auto [n, c] : cnt) {
        int w = (1LL * c * n * W + M / 2) / M;
        double p = 100.0 * c * n / C;
        lead += format("{:>18} {:>6.3f}% {}\n", n, p, string(w, '#'));
    }
    return lead;
}

template <typename T>
auto fmthist(const vector<T>& out) {
    if constexpr (is_floating_point_v<T>) {
        map<string, int> cnt;
        for (double f : out) {
            int n = floor(5 * f);
            cnt[format("{:6.2f}", n / 5.0)]++;
        }
        return fmthist(cnt);
    } else {
        map<T, int> cnt;
        for (const auto& f : out) {
            cnt[f]++;
        }
        return fmthist(cnt);
    }
}

template <typename T>
auto fmthistweighted(const vector<T>& out) {
    if constexpr (is_floating_point_v<T>) {
        map<double, int> cnt;
        for (double f : out) {
            int n = floor(5 * f);
            cnt[n / 5.0]++;
        }
        return fmthistweighted(cnt);
    } else {
        map<T, int> cnt;
        for (const auto& f : out) {
            cnt[f]++;
        }
        return fmthistweighted(cnt);
    }
}

template <typename T, typename Fn>
auto make(Fn&& fn) {
    vector<T> cnt;
    for (int runs = 0; runs < 10'000'000; runs++) {
        cnt.push_back(fn());
    }
    return fmthist(cnt);
}

template <typename T, typename Fn>
auto makew(Fn&& fn) {
    vector<T> cnt;
    for (int runs = 0; runs < 10'000'000; runs++) {
        cnt.push_back(fn());
    }
    return fmthistweighted(cnt);
}

void show_single_distributions() {
    println("Uniform:\n{}", //
            make<int>([]() { return rand_unif<int>(0, 20); }));

    println("Wide +1:\n{}", //
            make<int>([]() { return rand_wide<int>(0, 20, +1); }));
    println("Wide +5:\n{}", //
            make<int>([]() { return rand_wide<int>(0, 20, +5); }));

    println("Grav +5:\n{}", //
            make<int>([]() { return rand_grav<int>(0, 20, +5); }));
    println("Grav +1:\n{}", //
            make<int>([]() { return rand_grav<int>(0, 20, +1); }));
    println("Grav -1:\n{}", //
            make<int>([]() { return rand_grav<int>(0, 20, -1); }));
    println("Grav -5:\n{}", //
            make<int>([]() { return rand_grav<int>(0, 20, -5); }));

    println("Normal (0,5) for [-12,+12]:\n{}", //
            make<int>([]() { return rand_norm<int>(-12, +12, 0, 5); }));
    println("Normal (5,5) for [-12,+12]:\n{}", //
            make<int>([]() { return rand_norm<int>(-12, +12, 5, 5); }));
    println("Normal (5.5,5) for [-12,+12]:\n{}", //
            make<int>([]() { return rand_norm<int>(-12, +12, 5.5, 5); }));

    println("Peak 5 [0,20], +2:\n{}", //
            make<int>([]() { return rand_peak<int>(0, 20, 5, +2); }));
    println("Peak 5 [0,20], -2:\n{}", //
            make<int>([]() { return rand_peak<int>(0, 20, 5, -2); }));

    println("Exp c=+1 0..20:\n{}", //
            make<int>([]() { return rand_expo<int>(0, 20, 1.0); }));
    println("Exp c=+1 0..5:\n{}", //
            make<double>([]() { return rand_expo<double>(0, 5, 1.0); }));
    println("Exp c=-1 0..20:\n{}", //
            make<int>([]() { return rand_expo<int>(0, 20, -1.0); }));
    println("Exp c=-1 0..5:\n{}", //
            make<double>([]() { return rand_expo<double>(0, 5, -1.0); }));

    println("Geo p=+.2 0..20:\n{}", //
            make<int>([]() { return rand_geom<int>(0, 20, 0.2); }));
    println("Geo p=+.2 0..5:\n{}", //
            make<double>([]() { return rand_geom<double>(0, 5, 0.2); }));
    println("Geo p=-.2 0..20:\n{}", //
            make<int>([]() { return rand_geom<int>(0, 20, -0.2); }));
    println("Geo p=-.2 0..5:\n{}", //
            make<double>([]() { return rand_geom<double>(0, 5, -0.2); }));

    println("Ordered pairs:\n{}", //
            make<array<int, 2>>([&]() { return ordered_unif<int>(0, 5); }));
    println("Distinct pairs:\n{}", //
            make<array<int, 2>>([&]() { return diff_unif<int>(0, 6); }));
}

void show_weight_distributions() {
    double g20 = int_geom_prob_for_ratio(20, 20);
    double g30 = real_geom_prob_for_ratio(5, 30);
    double e40 = int_expo_base_for_ratio(20, 40);
    double e50 = real_expo_base_for_ratio(5, 50);
    debug(g20, g30, e40, e50);

    println("Geo r=20 0..20:\n{}", //
            make<int>([&]() { return rand_geom<int>(0, 20, g20); }));
    println("Geo r=30 0..5:\n{}", //
            make<double>([&]() { return rand_geom<double>(0, 5, g30); }));

    println("Exp r=40 0..20:\n{}", //
            make<int>([&]() { return rand_expo<int>(0, 20, e40); }));
    println("Exp r=50 0..5:\n{}", //
            make<double>([&]() { return rand_expo<double>(0, 5, e50); }));

    double ng20 = int_geom_prob_for_ratio(20, 1. / 20);
    double ng30 = real_geom_prob_for_ratio(5, 1. / 30);
    double ne40 = int_expo_base_for_ratio(20, 1. / 40);
    double ne50 = real_expo_base_for_ratio(5, 1. / 50);
    debug(ng20, ng30, ne40, ne50);

    println("Geo r=1/20 0..20:\n{}", //
            make<int>([&]() { return rand_geom<int>(0, 20, ng20); }));
    println("Geo r=1/30 0..5:\n{}", //
            make<double>([&]() { return rand_geom<double>(0, 5, ng30); }));

    println("Exp r=1/40 0..20:\n{}", //
            make<int>([&]() { return rand_expo<int>(0, 20, ne40); }));
    println("Exp r=1/50 0..5:\n{}", //
            make<double>([&]() { return rand_expo<double>(0, 5, ne50); }));

    for (int64_t n : {2LL, 10LL, 1000LL, 100000LL, 10'000'000LL, 1'000'000'000'000LL}) {
        double r = int_expo_base_for_ratio(n);
        println("int_expo_base(n) for n={:<13} {:.7f}", n, r);
    }

    double r40 = int_expo_base_for_ratio(40, 40);
    double g40 = int_geom_prob_for_ratio(40, 40);
    debug(r40, g40);

    println("Exponential weight 1..40, ratio 40:\n{}", //
            makew<int>([&]() { return rand_expo<int>(1, 40, r40); }));
    println("Geometric weight 1..40, ratio 40:\n{}", //
            makew<int>([&]() { return rand_geom<int>(1, 40, g40); }));

    double e5030 = int_expo_base_for_ratio(20, 50. / 30.);
    double e2012 = real_expo_base_for_ratio(8, 20. / 12.);
    debug(e5030, e2012);

    println("Exp r=50/30 30..50:\n{}", //
            makew<int>([&]() { return rand_expo<int>(30, 50, e5030); }));
    println("Exp r=20/12 12..20:\n{}", //
            makew<double>([&]() { return rand_expo<double>(12, 20, e2012); }));

    double rk1 = 1 / (M_E - 1);
    double rk3 = int_expo_base_for_ratio(3);
    double rk7 = int_expo_base_for_ratio(7);
    double rk10000 = int_expo_base_for_ratio(10000);
    println("converged for b=1: {}", rk1);
    println("converged for b=3: {}", rk3);
    println("converged for b=7: {}", rk7);
    println("converged for b=10000: {}\n", rk10000);

    println("Exp r={:.6f} 1..20:\n{}", rk1, //
            makew<int>([&]() { return rand_expo<int>(1, 20, rk1); }));
    println("Exp r={:.6f} 3..20:\n{}", rk3, //
            makew<int>([&]() { return rand_expo<int>(3, 20, rk3); }));
    println("Exp r={:.6f} 7..20:\n{}", rk7, //
            makew<int>([&]() { return rand_expo<int>(7, 20, rk7); }));
    println("Exp r={:.6f} 10000..10020:\n{}", rk10000, //
            makew<int>([&]() { return rand_expo<int>(10000, 10020, rk10000); }));
}

void show_sample_distributions() {
    println("3x ints from [0,7):\n{}", //
            make<vector<int>>([&]() { return int_sample<int>(3, 0, 7); }));
    println("4x ints from [0,7):\n{}", //
            make<vector<int>>([&]() { return int_sample<int>(4, 0, 7); }));

    println("3x choice from [0,4):\n{}",
            make<vector<array<int, 2>>>([&]() { return choose_sample<int>(3, 0, 4); }));
    println("3x pair from [0,2)x[0,3):\n{}", //
            make<vector<array<int, 2>>>(
                [&]() { return pair_sample<int>(3, 0, 2, 0, 3); }));
    println("3x distinct pair from [0,3):\n{}", //
            make<vector<array<int, 2>>>(
                [&]() { return distinct_pair_sample<int>(3, 0, 3); }));

    println("Partition 11 into 3:\n{}",
            make<vector<int>>([&]() { return partition_sample<int>(11, 3, 1); }));
}

void show_partition_distributions() {
    double rk1 = int_expo_base_for_ratio(1);

    println("Partial exponential partition:\n{}", //
            fmthist(rand_partial_partition(2'000'000, [&](int t) {
                return rand_expo<int>(1, min(t, 100'000), rk1);
            })));
    println("Partial normal partition:\n{}", //
            fmthist(rand_partial_partition(2'000'000, [&](int) {
                return rand_norm<int>(1, 100'000, 10'000, 6'000);
            })));
    println("Partial geometric partition:\n{}", //
            fmthist(rand_partial_partition(2'000'000, [&](int t) {
                return rand_geom<int>(1, min(t, 100'000), 0.02);
            })));
}

void show_compound_distributions() {
    println("Geometric sample:\n{}", //
            make<vector<int>>([&]() { return geom_sample(2, 0, 10, 0.5); }));
    println("Geometric sample:\n{}", //
            make<vector<int>>([&]() { return geom_sample(3, 0, 10, 0.5); }));
    println("Geometric sample:\n{}", //
            make<vector<int>>([&]() { return geom_sample(4, 0, 10, 0.5); }));

    println("Normal sample:\n{}", //
            make<vector<int>>([&]() { return norm_sample(3, 0, 9, 4, 3); }));

    println("Geometric sample:\n{}", //
            make<vector<int>>([&]() { return geom_sample(8, 0, 10, 0.5); }));
    println("Geometric sample:\n{}", //
            make<vector<int>>([&]() { return geom_sample(9, 0, 10, 0.5); }));
    println("Geometric sample:\n{}", //
            make<vector<int>>([&]() { return geom_sample(9, 0, 10, -0.5); }));
    println("Geometric sample:\n{}", //
            make<vector<int>>([&]() { return geom_sample(9, 0, 10, 1. / 3); }));
    println("Geometric sample:\n{}", //
            make<vector<int>>([&]() { return geom_sample(9, 0, 10, -1. / 3); }));
}

void speed_test_sampling() {
    vector<int> ms = {10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000};
    vector<int> ks = {2, 5, 15, 40, 420, 4000, 40000, 200000, 1500000, 7000000};

    vector<tuple<int, int>> inputs;
    for (int m : ms) {
        for (int k : ks) {
            if (k <= m) {
                inputs.emplace_back(m, k);
            }
        }
    }
    const auto runtime = 120'000ms / inputs.size();
    map<pair<int, int>, stringable> table;

    for (auto [m, k] : inputs) {
        START_ACC(ints);

        LOOP_FOR_DURATION_TRACKED_RUNS (runtime, now, runs) {
            print_time(now, runtime, "speed int_sample m={} k={} ({} runs)", m, k, runs);

            ADD_TIME_BLOCK(ints) {
                auto res = int_sample<int>(k, 0, m);
                assert(is_sorted(begin(res), end(res)));
                assert(int(res.size()) == k);
            }
        }

        table[{m, k}] = FORMAT_EACH(ints, runs * k);
    }

    print_time_table(table, "Distinct sampling");
}

int main() {
    RUN_BLOCK(speed_test_sampling());
    RUN_BLOCK(show_compound_distributions());
    return 0;
}
