#include "test_utils.hpp"
#include "hash.hpp"
#include "numeric/math.hpp"
#include "linear/matrix.hpp"
#include "numeric/partitions.hpp"

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

void stress_test_vec_sample(int n = 4096, int k = 37) {
    int start = 87632;
    vector<int> univ(n);
    iota(begin(univ), end(univ), start);
    shuffle(begin(univ), end(univ), mt);
    vector<int> cnt(n, 0);

    LOOP_FOR_DURATION_TRACKED (4s, now) {
        print_time(now, 4s, "stress test vec_sample");

        vector<int> sample = vec_sample(univ, k);
        for (int m : sample) {
            assert(start <= m && m < start + n);
            cnt[m - start]++;
        }
        assert(int(sample.size()) == k);
    }

    verify_normality(cnt);
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
    RUN_BLOCK(stress_test_vec_sample());
    RUN_BLOCK(stress_test_int_sample());
    RUN_BLOCK(stress_test_choose_sample());
    RUN_BLOCK(stress_test_pair_sample());
    return 0;
}
