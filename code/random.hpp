#pragma once

#include "hash.hpp"
#include "algo/sort.hpp"
#include "numeric/frac.hpp"

// *****

thread_local mt19937 mt(random_device{}());
using chard = uniform_int_distribution<char>;
using intd = uniform_int_distribution<int>;
using longd = uniform_int_distribution<long>;
using ulongd = uniform_int_distribution<size_t>;
using reald = uniform_real_distribution<double>;
using binomd = binomial_distribution<long>;
using geod = geometric_distribution<int>;
using normald = normal_distribution<double>;
using boold = bernoulli_distribution;

// *****

bool cointoss(double p) { return boold(p)(mt); }

template <typename T, typename O = T> // inclusive [a,b], uniform
O rand_unif(common_type_t<T> a, common_type_t<T> b) {
    if constexpr (is_integral_v<T>) {
        return uniform_int_distribution<T>(T(a), T(b))(mt);
    } else if constexpr (is_floating_point_v<T>) {
        return uniform_real_distribution<T>(T(a), T(b))(mt);
    } else {
        assert(false);
    }
    static_assert(is_integral_v<T> || is_floating_point_v<T>, "Invalid type T");
}

template <typename T, typename O = T> // inclusive [a,b], min/max
O rand_wide(common_type_t<T> a, common_type_t<T> b, int repulsion) {
    assert(-20 <= repulsion && repulsion <= 20);
    auto ans = rand_unif<T>(a, b);
    while (repulsion > 0)
        ans = max(ans, rand_unif<T>(a, b)), repulsion--;
    while (repulsion < 0)
        ans = min(ans, rand_unif<T>(a, b)), repulsion++;
    return ans;
}

template <typename T, typename O = T> // inclusive [a,b], normalish distribution
O rand_grav(common_type_t<T> a, common_type_t<T> b, int gravity) {
    assert(-20 <= gravity && gravity <= 20);
    auto ans = rand_unif<T, O>(a, b);
    auto mid = a + (b - a) / T(2);
    while (gravity > 0) {
        auto nxt = rand_unif<T, O>(a, b);
        ans = abs(ans - mid) <= abs(nxt - mid) ? ans : nxt, gravity--;
    }
    while (gravity < 0) {
        auto nxt = rand_unif<T, O>(a, b);
        ans = abs(ans - mid) >= abs(nxt - mid) ? ans : nxt, gravity++;
    }
    return ans;
}

template <typename T, typename O = T> // inclusive [a,b]
vector<O> rands_unif(int n, common_type_t<T> a, common_type_t<T> b) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_unif<T, O>(a, b);
    }
    return vec;
}

template <typename T, typename O = T> // inclusive [a,b]
vector<O> rands_wide(int n, common_type_t<T> a, common_type_t<T> b, int repulsion) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_wide<T, O>(a, b, repulsion);
    }
    return vec;
}

template <typename T, typename O = T> // inclusive [a,b]
vector<O> rands_grav(int n, common_type_t<T> a, common_type_t<T> b, int gravity) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_grav<T, O>(a, b, gravity);
    }
    return vec;
}

template <typename T, typename O = T> // inclusive [a,b], ordered pair u<=v
array<O, 2> ordered_unif(common_type_t<T> a, common_type_t<T> b) {
    auto x = rand_unif<T, O>(a, b), y = rand_unif<T, O>(a, b);
    return x <= y ? array<O, 2>{x, y} : array<O, 2>{y, x};
}

template <typename T, typename O = T> // inclusive [a,b], ordered pair u<=v
array<O, 2> ordered_wide(common_type_t<T> a, common_type_t<T> b, int repulsion) {
    auto x = rand_wide<T, O>(a, b, repulsion), y = rand_wide<T, O>(a, b, repulsion);
    return x <= y ? array<O, 2>{x, y} : array<O, 2>{y, x};
}

template <typename T, typename O = T> // inclusive [a,b], ordered pair u<=v
array<O, 2> ordered_grav(common_type_t<T> a, common_type_t<T> b, int gravity) {
    auto x = rand_grav<T, O>(a, b, gravity), y = rand_unif<T, O>(a, b, gravity);
    return x <= y ? array<O, 2>{x, y} : array<O, 2>{y, x};
}

template <typename T, typename O = T> // inclusive [a,b], returns !banned
O different(common_type_t<T> banned, common_type_t<T> a, common_type_t<T> b) {
    assert(a < b && (a != banned || a + 1 < b));
    if (a + 1 == b)
        return a;
    auto v = rand_unif<T, O>(a, b - 2);
    return v + !(v < banned);
}

template <typename T, typename O = T> // inclusive [a,b], ordered pair u<v
array<O, 2> different(common_type_t<T> a, common_type_t<T> b) {
    assert(a < b);
    auto x = rand_unif<T, O>(a, b), y = rand_unif<T, O>(a, b - 1);
    y += !(y < x);
    return x < y ? array<O, 2>{x, y} : array<O, 2>{y, x};
}

auto rand_string(int len, char a, char b) {
    uniform_int_distribution<char> dist(a, b);
    string str(len, '\0');
    for (int i = 0; i < len; i++) {
        str[i] = dist(mt);
    }
    return str;
}

auto rand_string_wide(int len, char a, char b, int repulsion) {
    assert(-20 <= repulsion && repulsion <= 20);
    uniform_int_distribution<char> dist(a, b);
    string str(len, '\0');
    for (int i = 0; i < len; i++) {
        str[i] = dist(mt);
        while (repulsion > 0)
            str[i] = max(str[i], dist(mt)), repulsion--;
        while (repulsion < 0)
            str[i] = min(str[i], dist(mt)), repulsion++;
    }
    return str;
}

auto rand_strings(int n, int minlen, int maxlen, char a, char b) {
    intd distlen(minlen, maxlen);
    vector<string> strs(n);
    for (int i = 0; i < n; i++) {
        strs[i] = rand_string(distlen(mt), a, b);
    }
    return strs;
}

/**
 * Generate a sorted sample of k distinct integers from the range [a..b)
 * It must hold that a <= b and k <= n = b - a.
 * Complexity: O(min(n, k log k))
 */
template <typename T = int, typename I = int>
auto int_sample(int k, I a, I b) {
    using sample_t = vector<T>;
    assert(k <= 100'000'000); // don't try anything crazy
    if (k == 0 || a >= b)
        return sample_t();

    long univ = b - a;
    assert(univ >= 0 && 0 <= k && k <= univ);

    // 1/5 One sample -- sample any
    if (k == 1) {
        uniform_int_distribution<I> distn(a, b - 1);
        sample_t sample = {distn(mt)};
        return sample;
    }

    // 2/5 Full sample -- run iota
    if (k == univ) {
        sample_t whole(univ);
        iota(begin(whole), end(whole), a);
        return whole;
    }

    // 3/5 Majority sample: run negative bitset sampling
    if (k >= univ / 2) {
        vector<bool> unsampled(univ, false);
        int included = univ;
        intd disti(0, univ - 1);
        while (included > k) {
            int i = disti(mt);
            included -= !unsampled[i];
            unsampled[i] = true;
        }
        sample_t sample(k);
        for (int i = 0, n = 0; i < k; n++) {
            if (!unsampled[n]) {
                sample[i++] = a + n;
            }
        }
        return sample;
    }

    // 4/5 Large minority sample: run positive bitset sampling
    if (k >= univ / 12) {
        vector<bool> sampled(univ, false);
        int included = 0;
        intd disti(0, univ - 1);
        while (included < k) {
            int i = disti(mt);
            included += !sampled[i];
            sampled[i] = true;
        }
        sample_t sample(k);
        for (int i = 0, n = 0; i < k; n++) {
            if (sampled[n]) {
                sample[i++] = a + n;
            }
        }
        return sample;
    }

    // 5/5 Large sample: run repeated sampling
    sample_t sample(k);
    uniform_int_distribution<I> dist(a, b - 1);
    for (int i = 0; i < k; i++) {
        sample[i] = dist(mt);
    }
    lsb_radix_sort(sample);
    int S = unique(begin(sample), end(sample)) - begin(sample);
    while (S < k) {
        int M = S;
        do {
            for (int i = M; i < k; i++) {
                sample[i] = dist(mt);
            }
            sort(begin(sample) + M, end(sample));
            inplace_merge(begin(sample) + S, begin(sample) + M, end(sample));
            M = unique(begin(sample) + S, end(sample)) - begin(sample);
        } while (M < k);

        inplace_merge(begin(sample), begin(sample) + S, end(sample));
        S = unique(begin(sample), end(sample)) - begin(sample);
    }
    return sample;
}

template <typename T = int, typename I = int>
auto int_sample_p(double p, I a, I b) {
    long ab = b - a;
    if (ab <= 100 || p > 0.20) {
        boold coind(p);
        vector<T> choice;
        for (auto n = a; n < b; n++)
            if (coind(mt))
                choice.push_back(n);
        return choice;
    }
    return int_sample<T>(binomd(ab, p)(mt), a, b);
}

/**
 * Generate a sorted sample of k integer pairs (x,y) where x, y are taken from the range
 * [a..b) and x < y.
 * It must hold that a <= b and k <= (n choose 2) where n = b - a.
 * Complexity: O(min(n choose 2, k log k))
 */
template <typename T = int, typename I>
auto choose_sample(int k, I a, I b) {
    using sample_t = vector<array<T, 2>>;
    assert(k <= 50'000'000); // don't try anything crazy
    if (k == 0 || a >= b - 1)
        return sample_t();

    long univ = 1L * (b - a) * (b - a - 1) / 2;
    assert(univ >= 0 && 0 <= k && k <= univ);

    // 1/5 One sample -- sample any
    if (k == 1) {
        uniform_int_distribution<I> distx(a, b - 1), disty(a, b - 2);
        I x = distx(mt), y = disty(mt);
        tie(x, y) = minmax(x + 0, y + (y >= x));
        sample_t sample = {{x, y}};
        return sample;
    }

    // 2/5 Full sample -- run iota
    if (k == univ) {
        sample_t whole(univ);
        int i = 0;
        for (I x = a; x < b; x++) {
            for (I y = x + 1; y < b; y++) {
                whole[i++] = {x, y};
            }
        }
        return whole;
    }

    // 3/5 Majority sample: run negative bitset sampling
    if (k >= univ / 2) {
        vector<bool> unsampled(univ, false);
        int included = univ;
        intd disti(0, univ - 1);
        while (included > k) {
            int i = disti(mt);
            included -= !unsampled[i];
            unsampled[i] = true;
        }
        sample_t sample(k);
        I x = a, y = a + 1;
        for (int i = 0, n = 0; i < k; n++) {
            if (!unsampled[n]) {
                sample[i++] = {x, y};
            }
            tie(x, y) = y == b - 1 ? make_pair(x + 1, x + 2) : make_pair(x, y + 1);
        }
        return sample;
    }

    // 4/5 Large minority sample: run positive bitset sampling
    if (k >= univ / 24) {
        vector<bool> sampled(univ, false);
        int included = 0;
        intd disti(0, univ - 1);
        while (included < k) {
            int i = disti(mt);
            included += !sampled[i];
            sampled[i] = true;
        }
        sample_t sample(k);
        I x = a, y = a + 1;
        for (int i = 0, n = 0; i < k; n++) {
            if (sampled[n]) {
                sample[i++] = {x, y};
            }
            tie(x, y) = y == b - 1 ? make_pair(x + 1, x + 2) : make_pair(x, y + 1);
        }
        return sample;
    }

    // 5/5 Large sample: run repeated sampling
    sample_t sample(k);
    uniform_int_distribution<I> distx(a, b - 1), disty(a, b - 2);
    for (int i = 0; i < k; i++) {
        I x = distx(mt), y = disty(mt);
        tie(x, y) = minmax(x + 0, y + (y >= x));
        assert(x < y);
        sample[i] = {x, y};
    }
    sort(begin(sample), end(sample));
    int S = unique(begin(sample), end(sample)) - begin(sample);
    while (S < k) {
        int M = S;
        do {
            for (int i = M; i < k; i++) {
                I x = distx(mt), y = disty(mt);
                tie(x, y) = minmax(x + 0, y + (y >= x));
                assert(x < y);
                sample[i] = {x, y};
            }
            sort(begin(sample) + M, end(sample));
            inplace_merge(begin(sample) + S, begin(sample) + M, end(sample));
            M = unique(begin(sample) + S, end(sample)) - begin(sample);
        } while (M < k);

        inplace_merge(begin(sample), begin(sample) + S, end(sample));
        S = unique(begin(sample), end(sample)) - begin(sample);
    }
    return sample;
}

template <typename T = int, typename I>
auto choose_sample_p(double p, I a, I b) {
    long ab = 1L * (b - a) * (b - a - 1) / 2;
    if (ab <= 100 || p > 0.20) {
        boold coind(p);
        vector<array<T, 2>> choice;
        for (auto x = a; x < b; x++)
            for (auto y = x + 1; y < b; y++)
                if (coind(mt))
                    choice.push_back({x, y});
        return choice;
    }
    return choose_sample<T>(binomd(ab, p)(mt), a, b);
}

/**
 * Generate a sorted sample of k integer pairs (x,y) where x is taken from the range
 * [a..b) and y is taken from the range [c..d).
 * It must hold that a <= b, c <= d, and k <= nm = (b - a)(d - c).
 * Complexity: O(min(nm, k log k))
 */
template <typename T = int, typename I>
auto pair_sample(int k, I a, I b, I c, I d) {
    using sample_t = vector<array<T, 2>>;
    assert(k <= 50'000'000); // don't try anything crazy
    if (k == 0 || a >= b || c >= d)
        return sample_t();

    long univ = 1L * (b - a) * (d - c);
    assert(univ >= 0 && 0 <= k && k <= univ);

    // 1/5 One sample -- sample any
    if (k == 1) {
        uniform_int_distribution<I> distx(a, b - 1), disty(c, d - 1);
        I x = distx(mt), y = disty(mt);
        sample_t sample = {{x, y}};
        return sample;
    }

    // 2/5 Full sample -- run iota
    if (k == univ) {
        sample_t whole(univ);
        int i = 0;
        for (I x = a; x < b; x++) {
            for (I y = c; y < d; y++) {
                whole[i++] = {x, y};
            }
        }
        return whole;
    }

    // 3/5 Majority sample: run negative bitset sampling
    if (k >= univ / 2) {
        vector<bool> unsampled(univ, false);
        int included = univ;
        intd disti(0, univ - 1);
        while (included > k) {
            int i = disti(mt);
            included -= !unsampled[i];
            unsampled[i] = true;
        }
        sample_t sample(k);
        I x = a, y = c;
        for (int i = 0, n = 0; i < k; n++) {
            if (!unsampled[n]) {
                sample[i++] = {x, y};
            }
            tie(x, y) = y == d - 1 ? make_pair(x + 1, c) : make_pair(x, y + 1);
        }
        return sample;
    }

    // 4/5 Large minority sample: run positive bitset sampling
    if (k >= univ / 24) {
        vector<bool> sampled(univ, false);
        int included = 0;
        intd disti(0, univ - 1);
        while (included < k) {
            int i = disti(mt);
            included += !sampled[i];
            sampled[i] = true;
        }
        sample_t sample(k);
        I x = a, y = c;
        for (int i = 0, n = 0; i < k; n++) {
            if (sampled[n]) {
                sample[i++] = {x, y};
            }
            tie(x, y) = y == d - 1 ? make_pair(x + 1, c) : make_pair(x, y + 1);
        }
        return sample;
    }

    // 5/5 Large sample: run repeated sampling
    sample_t sample(k);
    uniform_int_distribution<I> distx(a, b - 1), disty(c, d - 1);
    for (int i = 0; i < k; i++) {
        I x = distx(mt), y = disty(mt);
        sample[i] = {x, y};
    }
    sort(begin(sample), end(sample));
    int S = unique(begin(sample), end(sample)) - begin(sample);
    while (S < k) {
        int M = S;
        do {
            for (int i = M; i < k; i++) {
                I x = distx(mt), y = disty(mt);
                sample[i] = {x, y};
            }
            sort(begin(sample) + M, end(sample));
            inplace_merge(begin(sample) + S, begin(sample) + M, end(sample));
            M = unique(begin(sample) + S, end(sample)) - begin(sample);
        } while (M < k);

        inplace_merge(begin(sample), begin(sample) + S, end(sample));
        S = unique(begin(sample), end(sample)) - begin(sample);
    }
    return sample;
}

template <typename T = int, typename I>
auto pair_sample_p(double p, I a, I b, I c, I d) {
    long ab = b - a, cd = d - c;
    if (ab * cd <= 100 || p > 0.20) {
        boold coind(p);
        vector<array<T, 2>> choice;
        for (auto x = a; x < b; x++)
            for (auto y = c; y < d; y++)
                if (coind(mt))
                    choice.push_back({x, y});
        return choice;
    }
    return pair_sample<T>(binomd(ab * cd, p)(mt), a, b, c, d);
}

/**
 * Generate an unsorted sample of k integers pairs (x,y) where x and y are taken from the
 * range [a..b) and x != y.
 * It must hold that a <= b, and k <= n(n - 1) where n = b - a.
 * Complexity: O(min(n^2, k log k))
 */
template <typename T = int, typename I>
auto distinct_pair_sample(int k, I a, I b) {
    auto g = pair_sample(k, a, b, a, b - 1);
    for (auto& [u, v] : g)
        v += v >= u;
    return g;
}

template <typename T = int, typename I>
auto distinct_pair_sample_p(double p, I a, I b) {
    long ab = 1L * (b - a) * (b - a - 1);
    if (ab <= 100 || p > 0.20) {
        boold coind(p);
        vector<array<T, 2>> choice;
        for (auto x = a; x < b; x++)
            for (auto y = a; y < b; y++)
                if (x != y && coind(mt))
                    choice.push_back({x, y});
        return choice;
    }
    return distinct_pair_sample(binomd(ab, p)(mt), a, b);
}

/**
 * Run integer selection sampling over a vector, sampling k elements.
 * Complexity: O(k) and E[mt] = 3k.
 */
template <typename T>
auto vec_sample(const vector<T>& univ, int k) {
    int n = univ.size();
    assert(0 <= k && k <= n);
    vector<int> idx = int_sample(k, 0, n);
    vector<T> sample(k);
    for (int i = 0; i < k; i++)
        sample[i] = univ[idx[i]];
    return sample;
}

template <size_t k, typename T>
auto array_sample(const vector<T>& univ) {
    int n = univ.size();
    assert(0 < k && k <= n);
    vector<int> idx = int_sample(k, 0, n);
    array<T, k> sample;
    for (int i = 0; i < k; i++)
        sample[i] = univ[idx[i]];
    return sample;
}

/**
 * Generate an array of size n where each element parent[i] is selected
 * uniformly at random from [0..i-1] and parent[0] = 0.
 * Set first=1 to consider nodes [1..n] instead of [0..n)
 * Complexity: O(n)
 */
auto parent_sample(int n, int first = 0) {
    vector<int> parent(n + first, first);
    for (int i = 1 + first; i < n + first; i++) {
        intd dist(first, i - 1);
        parent[i] = dist(mt);
    }
    return parent;
}

/**
 * Generate a random partition of n into k parts each of size between m and M.
 * It must hold that k > 0 and 0 <= m <= M and mk <= n <= Mk.
 * Complexity: faster than linear
 * Not uniform, but close if M is sufficiently restrictive
 */
template <typename I = int>
auto partition_sample(I n, int k, I m = 1, I M = std::numeric_limits<I>::max()) {
    assert(n >= 0 && k > 0 && m >= 0 && m <= n / k && (n + k - 1) / k <= M);

    vector<I> parts(k, m);
    n -= m * k--;
    while (n > 0) {
        I add = (n + k) / (k + 1);
        int i = intd(0, k)(mt);
        if (parts[i] + add >= M) {
            n -= M - parts[i];
            parts[i] = M;
            swap(parts[i], parts[k--]);
        } else {
            n -= add;
            parts[i] += add;
        }
    }
    shuffle(begin(parts), end(parts), mt);
    return parts;
}

// Complexity: O(n) and generates a more balanced partition, naively
auto partition_sample_balanced(int n, int k, int m = 1, int M = INT_MAX) {
    assert(n >= 0 && k > 0 && m >= 0 && m <= n / k && (n + k - 1) / k <= M);

    vector<int> parts(k, m);
    n -= m * k--;
    while (n > 0) {
        const int add = 1;
        int i = intd(0, k)(mt);
        if (parts[i] + add >= M) {
            n -= M - parts[i];
            parts[i] = M;
            swap(parts[i], parts[k--]);
        } else {
            n -= add;
            parts[i] += add;
        }
    }
    shuffle(begin(parts), end(parts), mt);
    return parts;
}

/**
 * Generate a partition of n into k parts each of size between m_i and M_i.
 * It must hold that k > 0 and 0 <= m_i <= M_i and SUM(m_i) <= n <= SUM(M_i)
 * Complexity: faster than linear
 * Not uniform, but close if M is sufficiently restrictive
 */
template <typename I = int>
auto partition_sample(I n, int k, const vector<I>& m, const vector<I>& M) {
    assert(k > 0 && k <= int(m.size()) && k <= int(M.size()));

    vector<I> parts(k);
    vector<int> id(k);
    copy(begin(m), begin(m) + k--, begin(parts));
    iota(begin(id), end(id), 0);
    n -= accumulate(begin(parts), end(parts), I(0));
    while (n > 0) {
        I add = (n + k) / (k + 1);
        int i = intd(0, k)(mt), j = id[i];
        if (parts[j] + add >= M[j]) {
            n -= M[j] - parts[j];
            parts[j] = M[j];
            swap(id[i], id[k--]);
        } else {
            n -= add;
            parts[j] += add;
        }
    }
    return parts;
}

// Complexity: O(n) and generates a more balanced partition, naively
auto partition_sample_balanced(int n, int k, const vector<int>& m, const vector<int>& M) {
    assert(k > 0 && k <= int(m.size()) && k <= int(M.size()));

    vector<int> parts(k);
    vector<int> id(k);
    copy(begin(m), begin(m) + k--, begin(parts));
    iota(begin(id), end(id), 0);
    n -= accumulate(begin(parts), end(parts), 0);
    while (n > 0) {
        const int add = 1;
        int i = intd(0, k)(mt), j = id[i];
        if (parts[j] + add >= M[j]) {
            n -= M[j] - parts[j];
            parts[j] = M[j];
            swap(id[i], id[k--]);
        } else {
            n -= add;
            parts[j] += add;
        }
    }
    return parts;
}

/**
 * Like partition_sample but the first and last levels have size exactly 1.
 */
auto partition_sample_flow(int V, int ranks, int m = 1, int M = INT_MAX) {
    auto R = partition_sample_balanced(V - 2, ranks - 2, m, M);
    R.insert(R.begin(), 1);
    R.insert(R.end(), 1);
    return R;
}

/**
 * Like partition_sample but the first and last levels have size exactly 1.
 */
auto partition_sample_flow(int V, int ranks, const vector<int>& m, const vector<int>& M) {
    auto R = partition_sample_balanced(V - 2, ranks - 2, m, M);
    R.insert(R.begin(), 1);
    R.insert(R.end(), 1);
    return R;
}

/**
 * Generate a supply partition with n elements whose total sum is c and minimum is m.
 */
template <typename I = int>
auto supply_sample(int n, int positives, int negatives, I sum, I m = 1) {
    assert(n && positives && negatives && sum && positives + negatives <= n);
    vector<I> vec(n, 0);
    auto pos = partition_sample(sum, positives, m);
    auto neg = partition_sample(sum, negatives, m);
    auto idx = int_sample(positives + negatives, 0, n);
    shuffle(begin(idx), end(idx), mt);
    for (int i = 0; i < positives; i++)
        vec[idx[i]] = pos[i];
    for (int i = 0; i < negatives; i++)
        vec[idx[i + positives]] = -neg[i];
    return vec;
}
