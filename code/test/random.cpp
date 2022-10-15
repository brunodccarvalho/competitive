#include "test_utils.hpp"
#include "hash.hpp"
#include "numeric/math.hpp"
#include "linear/matrix.hpp"
#include "numeric/partitions.hpp"

template <typename T>
auto fmthist(const map<T, int>& cnt) {
    int M = 0, W = 150, C = 0, m = INT_MAX;
    vector<int> cnts;
    for (auto [n, c] : cnt) {
        M = max(M, c);
        m = min(m, c);
        C += c;
        cnts.push_back(c);
    }
    int S = cnts.size();
    string lead;
    lead += format("M/m: {:7.4f}\n", 1.0 * M / m);
    for (int i = 0; i + 1 < S; i++) {
        lead += format("{:>4.3f}{}", 1.0 * cnts[i] / cnts[i + 1], " \n"[i + 2 == S]);
    }
    for (auto [n, c] : cnt) {
        int w = (1LL * c * W + M / 2) / M;
        lead += format("{:>18} {:>6.3f}% {}\n", n, 100.0 * c / C, string(w, '#'));
    }
    return lead;
}

auto fmthist(vector<double>& out) {
    map<string, int> cnt;
    for (double f : out) {
        int n = floor(5 * f);
        cnt[format("{:6.2f}", n / 5.0)]++;
    }
    return fmthist(cnt);
}

void show_distributions() {
    auto make = [&](auto fn) {
        map<int, int> cnt;
        for (int runs = 0; runs < 10'000'000; runs++) {
            cnt[fn()]++;
        }
        return fmthist(cnt);
    };
    auto makev = [&](auto fn) {
        map<vector<int>, int> cnt;
        for (int runs = 0; runs < 10'000'000; runs++) {
            cnt[fn()]++;
        }
        return fmthist(cnt);
    };
    auto makevp = [&](auto fn) {
        map<vector<array<int, 2>>, int> cnt;
        for (int runs = 0; runs < 10'000'000; runs++) {
            cnt[fn()]++;
        }
        return fmthist(cnt);
    };
    auto makep = [&](auto fn) {
        map<array<int, 2>, int> cnt;
        for (int runs = 0; runs < 10'000'000; runs++) {
            cnt[fn()]++;
        }
        return fmthist(cnt);
    };
    auto maked = [&](auto fn) {
        vector<double> out;
        for (int runs = 0; runs < 10'000'000; runs++) {
            out.push_back(fn());
        }
        return fmthist(out);
    };

    println("Uniform:\n{}", make([]() { return rand_unif<int>(0, 20); }));

    println("Wide +1:\n{}", make([]() { return rand_wide<int>(0, 20, +1); }));
    println("Wide +5:\n{}", make([]() { return rand_wide<int>(0, 20, +5); }));

    println("Grav +5:\n{}", make([]() { return rand_grav<int>(0, 20, +5); }));
    println("Grav +1:\n{}", make([]() { return rand_grav<int>(0, 20, +1); }));
    println("Grav -1:\n{}", make([]() { return rand_grav<int>(0, 20, -1); }));
    println("Grav -5:\n{}", make([]() { return rand_grav<int>(0, 20, -5); }));

    println("=====================================================================");

    println("Exp c=+1 0..20:\n{}", make([]() { return rand_expo<int>(0, 20, 1.0); }));
    println("Exp c=+1 0..8:\n{}", maked([]() { return rand_expo<double>(0, 8, 1.0); }));
    println("Exp c=-1 0..20:\n{}", make([]() { return rand_expo<int>(0, 20, -1.0); }));
    println("Exp c=-1 0..8:\n{}", maked([]() { return rand_expo<double>(0, 8, -1.0); }));

    println("Geo p=+.2 0..20:\n{}", make([]() { return rand_geom<int>(0, 20, 0.2); }));
    println("Geo p=+.2 0..8:\n{}", maked([]() { return rand_geom<double>(0, 8, 0.2); }));
    println("Geo p=-.2 0..20:\n{}", make([]() { return rand_geom<int>(0, 20, -0.2); }));
    println("Geo p=-.2 0..8:\n{}", maked([]() { return rand_geom<double>(0, 8, -0.2); }));

    println("=====================================================================");

    double g20 = int_geom_prob_for_ratio(20, 20);
    double g30 = real_geom_prob_for_ratio(8, 30);
    double e40 = int_expo_base_for_ratio(20, 40);
    double e50 = real_expo_base_for_ratio(8, 50);
    debug(g20, g30, e40, e50);

    println("Geo r=20 0..20:\n{}", make([&]() { return rand_geom<int>(0, 20, g20); }));
    println("Geo r=30 0..8:\n{}", maked([&]() { return rand_geom<double>(0, 8, g30); }));

    println("Exp r=40 0..20:\n{}", make([&]() { return rand_expo<int>(0, 20, e40); }));
    println("Exp r=50 0..8:\n{}", maked([&]() { return rand_expo<double>(0, 8, e50); }));

    double ng20 = int_geom_prob_for_ratio(20, 1. / 20);
    double ng30 = real_geom_prob_for_ratio(8, 1. / 30);
    double ne40 = int_expo_base_for_ratio(20, 1. / 40);
    double ne50 = real_expo_base_for_ratio(8, 1. / 50);
    debug(ng20, ng30, ne40, ne50);

    println("Geo r=20 0..20:\n{}", make([&]() { return rand_geom<int>(0, 20, ng20); }));
    println("Geo r=30 0..8:\n{}", maked([&]() { return rand_geom<double>(0, 8, ng30); }));

    println("Exp r=40 0..20:\n{}", make([&]() { return rand_expo<int>(0, 20, ne40); }));
    println("Exp r=50 0..8:\n{}", maked([&]() { return rand_expo<double>(0, 8, ne50); }));

    println("=====================================================================");

    println("Ordered pairs:\n{}", makep([&]() { return ordered_unif<int>(0, 7); }));
    println("Distinct pairs:\n{}", makep([&]() { return diff_unif<int>(0, 7); }));

    println("Triple from [1,9):\n{}", makev([&]() { return int_sample<int>(3, 1, 9); }));
    println("Choose from [1,6):\n{}",
            makevp([&]() { return choose_sample<int>(3, 1, 5); }));
    println("Pair from [1,4)x[1,5):\n{}",
            makevp([&]() { return pair_sample<int>(3, 1, 4, 1, 5); }));
    println("Distinct pair from [1,5):\n{}",
            makevp([&]() { return distinct_pair_sample<int>(3, 1, 5); }));

    println("Partition 12 into 3:\n{}",
            makev([&]() { return partition_sample<int>(12, 3, 1); }));

    println("=====================================================================");
}

void verify_normality(vector<int>& v, bool show_histogram = false) {
    int n = v.size();
    int mi = min_element(begin(v), end(v)) - begin(v);
    int Mi = max_element(begin(v), end(v)) - begin(v);
    int m = v[mi], M = v[Mi];
    long sum = accumulate(begin(v), end(v), 0L);
    double mean = 1.0 * sum / n;
    double variance = 0;
    for (int k : v) {
        variance += (k - mean) * (k - mean);
    }
    variance /= n;
    double rho = sqrt(variance);
    unordered_map<int, int> within; // count numbers within k standard deviations
    for (int k : v) {
        within[int(abs(k - mean) / rho)]++;
    }
    for (int i = 1; i <= 5; i++) {
        within[i] += within[i - 1];
    }

    printcl("===== NORMALITY TEST =====\n");
    if (show_histogram) {
        long each = sum / n;
        for (int i = 0; i < n; i++) {
            print("{:2} -- {}\n", i, string(v[i] * 40 / each, '*'));
        }
    }
    print("sum: {}\n", sum);
    print("min: {}  ({:+.2f})  (i={})\n", m, (m - mean) / rho, mi);
    print("max: {}  ({:+.2f})  (i={})\n", M, (M - mean) / rho, Mi);
    print("mean: {:.2f}\n", mean);
    print("rho:  {:.2f}\n", rho);
    print("within 1 rho: {:5.2f}%\n", 100.0 * within[0] / n);
    print("within 2 rho: {:5.2f}%\n", 100.0 * within[1] / n);
    print("within 3 rho: {:5.2f}%\n", 100.0 * within[2] / n);
    print("within 4 rho: {:5.2f}%\n", 100.0 * within[3] / n);
    print("(remember, expected: 68.27 - 95.45 - 99.73)\n");
}

void stress_test_int_sample(int n = 1000) {
    vector<int> cnt(n, 0);
    intd distk(0, n - 1);

    LOOP_FOR_DURATION_TRACKED (4s, now) {
        print_time(now, 4s, "stress test int sample");

        int k = distk(mt);
        auto nums = int_sample(k, 0, n);
        assert(is_sorted(begin(nums), end(nums)));
        assert(int(nums.size()) == k);
        for (int i = 1; i < k; i++) {
            assert(nums[i - 1] < nums[i]);
        }
        for (int i : nums) {
            assert(0 <= i && i < n);
            cnt[i]++;
        }
    }

    verify_normality(cnt);
}

void stress_test_choose_sample(int n = 45) {
    vector<vector<int>> matcnt(n, vector<int>(n, 0));
    intd distk(0, n * (n - 1) / 2);

    LOOP_FOR_DURATION_TRACKED (4s, now) {
        print_time(now, 4s, "stress test choose sample");

        int k = distk(mt);
        auto nums = choose_sample(k, 0, n);
        assert(is_sorted(begin(nums), end(nums)));
        assert(int(nums.size()) == k);
        for (int i = 1; i < k; i++) {
            assert(nums[i - 1] < nums[i]);
        }
        for (auto [x, y] : nums) {
            assert(0 <= x && x < n);
            assert(0 <= y && y < n);
            assert(x < y);
            matcnt[x][y]++;
        }
    }

    vector<int> cnt;
    for (int x = 0; x < n; x++) {
        for (int y = x + 1; y < n; y++) {
            cnt.push_back(matcnt[x][y]);
        }
    }

    verify_normality(cnt);
}

void stress_test_pair_sample(int n = 31, int m = 31) {
    vector<vector<int>> matcnt(n, vector<int>(m, 0));
    intd distk(0, n * m);

    LOOP_FOR_DURATION_TRACKED (4s, now) {
        print_time(now, 4s, "stress test pair sample");

        int k = distk(mt);
        auto nums = pair_sample(k, 0, n, 0, m);
        assert(is_sorted(begin(nums), end(nums)));

        assert(int(nums.size()) == k);
        for (int i = 1; i < k; i++) {
            assert(nums[i - 1] < nums[i]);
        }
        for (auto [x, y] : nums) {
            assert(0 <= x && x < n);
            assert(0 <= y && y < m);
            matcnt[x][y]++;
        }
    }

    vector<int> cnt;
    for (int x = 0; x < n; x++) {
        for (int y = 0; y < m; y++) {
            cnt.push_back(matcnt[x][y]);
        }
    }

    verify_normality(cnt);
}

int main() {
    RUN_BLOCK(show_distributions());
    return 0;
    RUN_BLOCK(stress_test_int_sample());
    RUN_BLOCK(stress_test_choose_sample());
    RUN_BLOCK(stress_test_pair_sample());
    return 0;
}
