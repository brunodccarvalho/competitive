#include "test_utils.hpp"
#include "struct/pbds.hpp"

void speed_test_compression() {
    vector<int> Ns = {3000, 50000, 100000, 200000, 300000, 500000, 1000000, 2000000};

    map<pair<string, int>, stringable> times;

    for (int N : Ns) {
        START_ACC3(sort, map, lower_bound);
        START_ACC3(inline, unordered_map, gp_hash_table);

        vector<int> nums = rands_unif<int>(N, -1e9, 1e9);
        vector<int> a = nums, b = nums, c = nums, d = nums, e(N);

        ADD_TIME_BLOCK(inline) {
            vector<pair<int, int>> x(N);
            for (int i = 0; i < N; i++) {
                x[i] = {nums[i], i};
            }
            sort(begin(x), end(x));
            e[x[0].second] = 0;
            for (int i = 1; i < N; i++) {
                e[x[i].second] = e[x[i - 1].second] + (x[i].first != x[i - 1].first);
            }
        }

        ADD_TIME_BLOCK(sort) {
            sort(begin(nums), end(nums));
            nums.erase(unique(begin(nums), end(nums)), end(nums));
        }

        int M = nums.size();

        ADD_TIME_BLOCK(lower_bound) {
            for (int i = 0; i < N; i++) {
                a[i] = lower_bound(begin(nums), end(nums), a[i]) - begin(nums);
            }
        }

        ADD_TIME_BLOCK(map) {
            map<int, int> mm;
            for (int i = 0; i < M; i++) {
                mm.emplace_hint(mm.end(), nums[i], i);
            }
            for (int i = 0; i < N; i++) {
                b[i] = mm.at(b[i]);
            }
        }

        ADD_TIME_BLOCK(unordered_map) {
            unordered_map<int, int> mm;
            mm.reserve(N);
            for (int i = 0; i < M; i++) {
                mm.emplace(nums[i], i);
            }
            for (int i = 0; i < N; i++) {
                c[i] = mm.at(c[i]);
            }
        }

        ADD_TIME_BLOCK(gp_hash_table) {
            hash_map<int, int> mm;
            for (int i = 0; i < M; i++) {
                mm[nums[i]] = i;
            }
            for (int i = 0; i < N; i++) {
                d[i] = mm[d[i]];
            }
        }

        assert(a == b && a == c && a == d && a == e);

        times[{"sort", N}] = FORMAT_TIME(sort);
        times[{"map", N}] = FORMAT_TIME(map);
        times[{"inline", N}] = FORMAT_TIME(inline);
        times[{"lower_bound", N}] = FORMAT_TIME(lower_bound);
        times[{"unordered_map", N}] = FORMAT_TIME(unordered_map);
        times[{"gp_hash_table", N}] = FORMAT_TIME(gp_hash_table);
    }

    print_time_table(times, "Coordinate compression (-1e9..1e9)");
}

int main() {
    RUN_BLOCK(speed_test_compression());
    return 0;
}
