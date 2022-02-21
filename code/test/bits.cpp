#include "test_utils.hpp"
#include "numeric/bits.hpp"
#include "algo/y_combinator.hpp"

void unit_test_bits() {
    assert(lsbits(7, 3) == "111");
    assert(lsbits(13, 4) == "1011");
    assert(lsbits(0b1001001, 7) == "1001001");
    assert(lsbits(0b101011010110, 12) == "011010110101");
    assert(lsbits(0, 7) == "0000000");
    assert(lsbits(32, 7) == "0000010");

    assert(msbits(13, 4) == "1101");
    assert(msbits(0b1001001, 7) == "1001001");
    assert(msbits(0b101011010110, 12) == "101011010110");
    assert(msbits(32, 7) == "0100000");

    unsigned v = 0b10000001000001000010001001011111;
    reverse_bits(v);
    assert(v == 0b11111010010001000010000010000001);
}

void unit_test_foreach_mask() {
    constexpr int b = 4, n = 6;

    vector<unsigned> masks;
    for (int i = 0; i < (1 << n); i++) {
        if (__builtin_popcount(i) == b) {
            masks.push_back(i);
        }
    }

    vector<unsigned> loop;
    FOR_EACH_MASK (mask, b, n) { loop.push_back(mask); }
    assert(loop == masks);
}

void stress_test_gray_code() {
    constexpr int n = 15;

    vector<vector<int>> table(1);
    for (int i = 0; i <= n; i++) {
        table[0].push_back(i - 1);
    }

    {
        vector<int> cnt_in(n + 1), cnt_out(n + 1);
        FOR_ALL_MASKS_GRAY_CODE(mask, n, in, out) {
            cnt_in[in + 1]++, cnt_out[out + 1]++;
        }

        table.push_back(cnt_in);
        table.push_back(cnt_out);
    }

    print("In/Out frequencies\n{}", mat_to_string(table));
}

void speed_test_gray_code() {
    constexpr int n = 25, N = 1 << n;
    vector<int> a(n);
    for (int i = 0; i < n; i++) {
        a[i] = 1 << i;
    }

    vector<int> naive(N);
    TIME_BLOCK(naive) {
        for (int mask = 0; mask < N; mask++) {
            FOR_EACH_BIT_NUMBER (bit, i, mask) { naive[mask] += a[i]; }
        }
    }

    vector<int> comb(N);
    TIME_BLOCK(dfs) {
        y_combinator([&](auto self, int mask, int sum, int i) -> void {
            if (i == n) {
                comb[mask] = sum;
            } else {
                self(mask, sum, i + 1);
                self(mask | (1 << i), sum + a[i], i + 1);
            }
        })(0, 0, 0);
    }

    vector<int> fast(N);
    TIME_BLOCK(fast) {
        int sum = 0;
        FOR_ALL_MASKS_GRAY_CODE(mask, n, in, out) {
            if (in != -1) {
                sum += a[in];
            } else if (out != -1) {
                sum -= a[out];
            }
            fast[mask] = sum;
        }
    }

    assert(naive == fast);
    assert(naive == comb);
}

int main() {
    RUN_SHORT(speed_test_gray_code());
    RUN_SHORT(stress_test_gray_code());
    RUN_SHORT(unit_test_bits());
    RUN_SHORT(unit_test_foreach_mask());
    return 0;
}
