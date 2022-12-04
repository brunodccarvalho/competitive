#include "test_utils.hpp"
#include "hash.hpp"
#include "numeric/math.hpp"
#include "linear/matrix.hpp"
#include "numeric/partitions.hpp"

template <typename T, typename U>
auto fmthist(const vector<pair<T, U>>& cnt) {
    const int max_bars = 150;
    using V = conditional_t<is_integral_v<U>, int64_t, double>;
    V M = 0, C = 0, m = numeric_limits<U>::max();
    vector<U> cnts;
    for (auto [n, c] : cnt) {
        M = max<V>(M, c);
        m = min<V>(m, c);
        C += c;
        cnts.push_back(c);
    }
    int S = cnts.size();
    string lead;
    lead += format("C={}, S={}, M/m={:f}\n", C, S, 1.0 * M / m);
    for (int i = 0; i + 1 < S && S <= 40; i++) {
        lead += format("{:>4.3f}{}", 1.0 * cnts[i] / cnts[i + 1], " \n"[i + 2 == S]);
    }
    for (auto [n, c] : cnt) {
        int bars = floor((1.0 * c * max_bars + M / 2) / M);
        double p = 100.0 * c / C;
        lead += format("{:>18} {:>6.3f}% {}\n", n, p, string(bars, '#'));
    }
    return lead;
}

template <typename T>
auto group(const vector<T>& out, int d = 5) {
    if constexpr (is_floating_point_v<T>) {
        map<double, int> cnt;
        for (double f : out) {
            cnt[round(f * d) / d]++;
        }
        vector<pair<string, int>> cnts;
        for (auto [f, c] : cnt) {
            cnts.emplace_back(format("{:+6.2f}", f), c);
        }
        return fmthist(cnts);
    } else {
        map<T, int> cnt;
        for (const auto& f : out) {
            cnt[f]++;
        }
        vector<pair<T, int>> cnts(begin(cnt), end(cnt));
        return fmthist(cnts);
    }
}

template <typename T>
auto groupw(const vector<T>& out, int d = 5) {
    if constexpr (is_floating_point_v<T>) {
        map<double, double> cnt;
        for (double f : out) {
            cnt[round(f * d) / d] += abs(f);
        }
        vector<pair<string, double>> cnts;
        for (auto [f, c] : cnt) {
            cnts.emplace_back(format("{:+6.2f}", f), c);
        }
        return fmthist(cnts);
    } else {
        map<T, int64_t> cnt;
        for (T f : out) {
            cnt[f] += f;
        }
        vector<pair<T, int64_t>> cnts;
        for (auto [f, c] : cnt) {
            cnts.emplace_back(f, c);
        }
        return fmthist(cnts);
    }
}

template <typename T, typename Fn>
auto make(Fn&& fn, int d = 5) {
    vector<T> cnt;
    for (int runs = 0; runs < 10'000'000; runs++) {
        cnt.push_back(fn());
    }
    return group(cnt, d);
}

template <typename T, typename Fn>
auto makew(Fn&& fn, int d = 5) {
    vector<T> cnt;
    for (int runs = 0; runs < 10'000'000; runs++) {
        cnt.push_back(fn());
    }
    return groupw(cnt, d);
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

    println("Peak 5 [0,25], +1:\n{}", //
            make<int>([]() { return rand_peak<int>(0, 25, 5, +1); }));
    println("Peak 9 [0,25], -1:\n{}", //
            make<int>([]() { return rand_peak<int>(0, 25, 9, -1); }));
    println("Peak 5 [0,20], +2:\n{}", //
            make<int>([]() { return rand_peak<int>(0, 20, 5, +2); }));
    println("Peak 5 [0,20], -2:\n{}", //
            make<int>([]() { return rand_peak<int>(0, 20, 5, -2); }));

    println("Exp 1..20:\n{}", //
            make<int>([]() { return rand_expo<int>(1, 20); }));
    println("Exp 1..5:\n{}", //
            make<double>([]() { return rand_expo<double>(1, 5); }));

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
    println("Exp 1..20:\n{}", //
            makew<int>([&]() { return rand_expo<int>(1, 20); }));
    println("Exp 3..20:\n{}", //
            makew<int>([&]() { return rand_expo<int>(3, 20); }));
    println("Exp 7..20:\n{}", //
            makew<int>([&]() { return rand_expo<int>(7, 20); }));
    println("Exp 200..230:\n{}", //
            makew<int>([&]() { return rand_expo<int>(200, 230); }));
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
    println("Partial exponential partition:\n{}", //
            group(rand_partial_partition(2'000'000, [&](int t) {
                return rand_expo<int>(1, min(t, 100'000));
            })));
    println("Partial normal partition:\n{}", //
            group(rand_partial_partition(2'000'000, [&](int) {
                return rand_norm<int>(1, 100'000, 10'000, 6'000);
            })));
    println("Partial geometric partition:\n{}", //
            group(rand_partial_partition(2'000'000, [&](int t) {
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
    RUN_BLOCK(show_single_distributions());
    RUN_BLOCK(show_weight_distributions());
    RUN_BLOCK(show_partition_distributions());
    RUN_BLOCK(speed_test_sampling());
    return 0;
}
