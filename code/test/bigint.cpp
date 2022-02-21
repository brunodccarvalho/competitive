#include "test_utils.hpp"
#include "numeric/bigint.hpp"
#include "lib/bigint_utils.hpp"

inline namespace detail {

constexpr unsigned U = UINT_MAX;         // 0xffffffff
constexpr unsigned M = UINT_MAX / 2 + 1; // 0x80000000

reald distp(0.0, 1.0);
ulongd distv(0, U), distvp(1, U);
intd distn_small(0, 10), distn_pos(1, 8), distn_large(50, 300), distn_any(0, 40);
boold distneg(0.5);

string trim_numeric_string(string s) {
    int S = s.size(), l = 0, r = S;
    while (r > 0 && isspace(s[r - 1]))
        r--;
    s.erase(r);
    while (l < S && (isspace(s[l]) || s[l] == '+'))
        l++;
    s.erase(0, l);
    return s;
}

template <int m>
array<int, m> random_ints(intd& dist) {
    array<int, m> arr;
    for (int i = 0; i < m; i++)
        arr[i] = dist(mt);
    return arr;
}

template <int m>
array<bigint, m> random_bigints(array<int, m> ns, int base = 10, double neg_p = 0.0) {
    array<bigint, m> arr;
    for (int i = 0; i < m; i++)
        arr[i] = random_bigint(ns[i], base, neg_p);
    return arr;
}

} // namespace detail

/**
 * It should be easy and correct to mix up ints and bigints
 * Mixing up long and bigint not so, as bigint replaces long usages.
 */
void minimum_usability_test() {
    bigint u = 0, v = 73, w = -73;
    assert(u == v + w);
    assert(v + w == 0);

    u = 46;
    v = -32;
    w = -14;
    assert(u + v + w == 0 && u + v + w >= 0 && u + v + w <= 0);

    w = -18;
    assert(u + v + w < 0 && u + v + w > -10);
    assert(abs(u + v + w) == 4);
}

void unit_test_add() {
    bigint u, v, a, b, c;

    for (int i = 0; i < 1'000'000; i++)
        u += 2 * i + 1, v -= 2 * i + 1;
    assert(u == bigint("1000000000000"));
    assert(v == bigint("-1000000000000"));

    u.nums = {M, U, U, U};
    u += M;
    a.nums = {0, 0, 0, 0, 1};
    assert(u == a);

    u.nums = {M, U, U - 1, U};
    u += M;
    a.nums = {0, 0, U, U};
    assert(u == a);

    a = "0123456789012345678901234567890123456789"s;
    b = "9876543210987654321098765432109876543210"s;
    c = "9999999999999999999999999999999999999999"s;
    u = a + b;
    assert(u == c);
    u -= b;
    assert(u == a);
}

void unit_test_sub() {
    bigint u, a, b, c;

    vector<int> nums;
    for (int i = 0; i < 100000; i++)
        nums.push_back(!(i & 1) ? 1 + 2 * i : 1 - 2 * i);
    shuffle(begin(nums), end(nums), mt);
    for (int n : nums)
        u += n;
    assert(u == 0);
    shuffle(begin(nums), end(nums), mt);
    for (int n : nums)
        u -= n;
    assert(u == 0);

    u.nums = {7, 0, 0, 0, 1, 2};
    a.nums = {U, U, U, U, 0, 2};
    b = u;
    u -= 8;
    assert(u == a);
    b -= a;
    assert(b == 8);
}

void unit_test_mul() {
    const bigint fac40("815915283247897734345611269596115894272000000000");
    bigint u = 1, v = 1, w = 1;
    for (int i = 1; i <= 40; i++)
        u *= i, v *= 2 * i, w *= 16 * i;
    assert(u == fac40);
    assert(v == fac40 << 40);
    assert(w == fac40 << 160);
    assert(v == w >> 120);
    assert(fac40 % 41 == 40);

    v = bigint("660955782884386677434829685779361532098606832525794499"
               "673096513026019562749349063704800410525656374299407003"
               "7769599882399012397170569200279466412758131334001");
    u = 1;
    for (int i = 1; i <= 100; i++)
        u *= 37;
    assert(u == v);
}

void unit_test_div() {
    bigint a, b, c, d, x;
    a.nums = {0, 0, 0, 4};
    b.nums = {0, 2};
    c.nums = {0, 0, 2};
    d = a / b;
    assert(c == d);

    a = "123456789123456789123456789123456789"s;
    b = "987654321987654321"s;
    c = "124999998860937500"s;
    d = "137519289137519289"s;
    x = div_mod(a, b);
    assert(a == c && x == d);
}

void unit_test_shift() {
    string s = "101011101100001101010101000001101100001111110101";
    string z = s + string(150, '0');
    bigint v(s, 2);
    int m = s.length();

    for (int i = 0; i < 128; i++) {
        bigint u = v << i;
        bigint w(z.substr(0, m + i), 2);
        assert(u == w);
    }
    for (int i = 0; i < 140; i++) {
        bigint u = (v << 127) >> i;
        bigint w(z.substr(0, m + (127 - i)), 2);
        assert(u == w);
    }
}

void unit_test_increment() {
    string s = "1001010111000101010";
    auto b = bigint(s, 2);
    assert(++b == bigint("1001010111000101011", 2));
    assert(++b == bigint("1001010111000101100", 2));
    assert(++b == bigint("1001010111000101101", 2));
    assert(++b == bigint("1001010111000101110", 2));
    assert(--b == bigint("1001010111000101101", 2));
    assert(--b == bigint("1001010111000101100", 2));
    assert(--b == bigint("1001010111000101011", 2));
    assert(--b == bigint("1001010111000101010", 2));
    assert(b++ == bigint("1001010111000101010", 2));
    assert(b++ == bigint("1001010111000101011", 2));
    assert(b++ == bigint("1001010111000101100", 2));
    assert(b-- == bigint("1001010111000101101", 2));
    assert(b-- == bigint("1001010111000101100", 2));
    assert(b-- == bigint("1001010111000101011", 2));
    b = 3;
    assert(b-- == 3);
    assert(b-- == 2);
    assert(b-- == 1);
    assert(b-- == 0);
    assert(b-- == -1);
    assert(++b == -1);
    assert(++b == 0);
    assert(++b == 1);
    assert(++b == 2);
}

void unit_test_print() {
    vector<string> strs = {
        "123456789012345678901234567890 ",
        "12121212121212121212121212",
        "  -111222333444555666777888999000  ",
        "+123456789",
        "+987654321   ",
        "12345",
        "-54321",
        "123456789012345  ",
        "-987654321012345",
        "   -9999999999999999999999999999999999999  ",
        "1000000000000000000000000000000000000",
    };
    for (const auto& str : strs) {
        bigint u(str);
        assert(to_string(u) == trim_numeric_string(str));
    }
}

void unit_test_sqrt() {
    bigint u = bigpow(18), v = bigpow(28) + 73, w = bigpow(12) + 12;
    assert(sqrt(u) == bigpow(9));
    assert(sqrt(v) == bigpow(14));
    assert(sqrt(w) == bigpow(6));
}

ms stress_runtime = 2s;

void stress_test_sqrt() {
    intd digitsd(10, 500);

    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test sqrt");

        auto n = random_numeric_string(digitsd(mt), 10);
        auto u = sqrt(bigint(n));

        assert(u * u <= n);
        assert(n < (u + 1) * (u + 1));
    }
}

void stress_test_to_string() {
    intd digitsd(10, 500);

    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test base to_string");

        for (int b = 2; b <= 10; b++) {
            auto s = random_numeric_string(digitsd(mt), b, 0.3);
            auto z = trim_numeric_string(s);
            bigint u(s, b);
            auto msb = msbits(u), lsb = lsbits(u);
            assert(to_string(u, b) == z);
            assert(bigint(msb, 2) == u);

            msb.erase(0, 1), lsb.erase(0, 1);
            reverse(begin(lsb), end(lsb));
            assert(msb == lsb);
        }
    }
}

void stress_test_compare_sort() {
    intd distN(100, 2000);

    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test compare sort");

        int N = distN(mt);
        vector<bigint> ints(N);
        intd digitsd(40, 60);
        for (int i = 0; i < N; i++) {
            ints[i] = bigint(random_numeric_string(digitsd(mt), 10, 0.3));
        }
        sort(begin(ints), end(ints));
        for (int i = 0; i + 1 < N; i++) {
            bigint dif = ints[i + 1] - ints[i];
            assert(dif.sign == 0 && dif >= 0);
        }
    }
}

void stress_test_add_commutative() {
    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test add commutative");
        auto [a, b] = random_bigints<2>(random_ints<2>(distn_small));
        bigint c = a + b;
        bigint d = b + a;
        assert(c == d);
    }
}

void stress_test_add_transitive() {
    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test add transitive");
        auto [a, b, c] = random_bigints<3>(random_ints<3>(distn_small));
        bigint d = (a + b) + c;
        bigint e = a + (b + c);
        assert(d == e);
    }
}

void stress_test_add_sub_reverse() {
    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test add sub reverse");
        auto [a, b] = random_bigints<2>(random_ints<2>(distn_small));
        bigint c = a - b;
        bigint d = b + c;
        assert(a == d);
    }
}

void stress_test_add_sub_group() {
    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test add sub group");
        auto [a, b, c] = random_bigints<3>(random_ints<3>(distn_small));
        bigint d = a - b + c;
        bigint e = a - (b - c);
        bigint f = a - b - c;
        bigint g = a - (b + c);
        assert(d == e && f == g);
    }
}

void stress_test_mul_commutative() {
    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test mul commutative");
        auto [a, b] = random_bigints<2>(random_ints<2>(distn_small));
        bigint c = a * b;
        bigint d = b * a;
        assert(c == d);
    }
}

void stress_test_mul_transitive() {
    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test mul transitive");
        auto [a, b, c] = random_bigints<3>(random_ints<3>(distn_small));
        bigint d = (a * b) * c;
        bigint e = a * (b * c);
        assert(d == e);
    }
}

void stress_test_mul_distributive() {
    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test mul distributive");
        auto [a, b, c] = random_bigints<3>(random_ints<3>(distn_small));
        bigint d = a * (b + c);
        bigint e = a * b + a * c;
        assert(d == e);
    }
}

void stress_test_div_perfect() {
    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test div perfect");
        auto [a, b] = random_bigints<2>(random_ints<2>(distn_pos), 10, 0.0);
        bigint c = a * b;
        bigint d = c / a;
        assert(d == b);
    }
}

void stress_test_div_imperfect() {
    LOOP_FOR_DURATION_TRACKED (stress_runtime, now) {
        print_time(now, stress_runtime, "stress test div imperfect");
        auto [a, b] = random_bigints<2>(random_ints<2>(distn_pos));
        bigint q = a;
        bigint r = div_mod(q, b);
        assert(q * b + r == a && magnitude_cmp(r, b));
    }
}

void speed_test_pairwise_mul(int max_scale = 16) {
    const int RUNS = max_scale;
    const auto runtime = 30000ms / RUNS;

    map<pair<string, int>, stringable> table;

    for (int scale = 1; scale <= max_scale; scale++) {
        START_ACC(mul);
        LOOP_FOR_DURATION_OR_RUNS_TRACKED (runtime, now, 10000, runs) {
            print_time(now, runtime, "speed test multiplication scale={}", scale);

            auto a = random_bigint(1 << scale);
            auto b = random_bigint(1 << scale);

            START(mul);
            auto c = a * b;
            ADD_TIME(mul);
        }

        table[{"bigint", 1 << scale}] = FORMAT_EACH(mul, runs);
        table[{"runs", 1 << scale}] = runs;
    }

    print_time_table(table, "Bigint multiplication");
}

int main() {
    RUN_SHORT(minimum_usability_test());
    RUN_SHORT(unit_test_add());
    RUN_SHORT(unit_test_sub());
    RUN_SHORT(unit_test_mul());
    RUN_SHORT(unit_test_div());
    RUN_SHORT(unit_test_shift());
    RUN_SHORT(unit_test_increment());
    RUN_SHORT(unit_test_print());
    RUN_SHORT(unit_test_sqrt());

    RUN_BLOCK(speed_test_pairwise_mul());

    RUN_SHORT(stress_test_sqrt());
    RUN_SHORT(stress_test_to_string());
    RUN_SHORT(stress_test_compare_sort());
    RUN_SHORT(stress_test_add_commutative());
    RUN_SHORT(stress_test_add_transitive());
    RUN_SHORT(stress_test_add_sub_reverse());
    RUN_SHORT(stress_test_add_sub_group());
    RUN_SHORT(stress_test_mul_commutative());
    RUN_SHORT(stress_test_mul_transitive());
    RUN_SHORT(stress_test_mul_distributive());
    RUN_SHORT(stress_test_div_perfect());
    RUN_SHORT(stress_test_div_imperfect());
    return 0;
}
