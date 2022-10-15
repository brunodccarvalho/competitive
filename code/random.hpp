#pragma once

#include "hash.hpp"

// *****

thread_local mt19937 mt(random_device{}());
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

template <typename T, typename O = T> // ans=[a,b], uniform
O rand_unif(common_type_t<T> a, common_type_t<T> b) {
    if constexpr (is_integral_v<T>) {
        return uniform_int_distribution<T>(T(a), T(b))(mt);
    } else if constexpr (is_floating_point_v<T>) {
        return uniform_real_distribution<T>(T(a), T(b))(mt);
    } else {
        assert(false);
    }
}

template <typename T, typename O = T> // ans=[a,b], uniform, <0=>min, >0=>max
O rand_wide(common_type_t<T> a, common_type_t<T> b, int draw) {
    assert(-20 <= draw && draw <= 20);
    auto ans = rand_unif<T>(a, b);
    while (draw > 0)
        ans = max(ans, rand_unif<T>(a, b)), draw--;
    while (draw < 0)
        ans = min(ans, rand_unif<T>(a, b)), draw++;
    return ans;
}

template <typename T, typename O = T> // ans=[a,b], uniform, <0=>vee, >0=>normal
O rand_grav(common_type_t<T> a, common_type_t<T> b, int grav) {
    assert(-20 <= grav && grav <= 20);
    auto ans = rand_unif<T>(a, b);
    while (grav > 0) {
        auto nxt = rand_unif<T>(a, b);
        ans = max(ans - a, b - ans) <= max(nxt - a, b - nxt) ? ans : nxt, grav--;
    }
    while (grav < 0) {
        auto nxt = rand_unif<T>(a, b);
        ans = max(ans - a, b - ans) >= max(nxt - a, b - nxt) ? ans : nxt, grav++;
    }
    return ans;
}

template <typename T, typename O = T> // ans=[a,b], exponential, slope 1/(c+i), c!=0
O rand_expo(common_type_t<T> a, common_type_t<T> b, double c) {
    if (a >= b) {
        return a;
    } else if (c < 0) {
        return b - rand_expo<T, O>(a, b, -c) + a;
    } else if constexpr (is_integral_v<T>) {
        double e = rand_unif<double>(log(c), log(b - a + c + 1.0));
        return clamp(T(a + exp(e) - c), T(a), T(b));
    } else if constexpr (is_floating_point_v<T>) {
        double e = rand_unif<double>(log(c), log(b - a + c));
        return clamp(T(a + exp(e) - c), T(a), T(b));
    } else {
        assert(false);
    }
}

template <typename T, typename O = T> // ans=[a,b], geometric, slope 1-p, -1<p<1, p!=0
O rand_geom(common_type_t<T> a, common_type_t<T> b, double p) {
    if (a >= b) {
        return a;
    } else if (p < 0.0) {
        return b - rand_geom<T, O>(a, b, -p) + a;
    } else if constexpr (is_integral_v<T>) {
        double M_log_1_p = log1p(-p);
        double largest = 1.0 - exp(M_log_1_p * (b + 1.0 - a));
        double cand = a + log1p(-rand_unif<double>(0, largest)) / M_log_1_p;
        return clamp(T(cand), T(a), T(b));
    } else if constexpr (is_floating_point_v<T>) {
        double M_log_1_p = log1p(-p);
        double largest = 1.0 - exp(M_log_1_p * (b - a));
        double cand = a + log1p(-rand_unif<double>(0, largest)) / M_log_1_p;
        return clamp(T(cand), T(a), T(b));
    } else {
        assert(false);
    }
}

// Find c for rand_expo<real>(0,n,c) so that freq(0)/freq(n)=r
double real_expo_base_for_ratio(double n, double r) {
    assert(r > 0);
    return n <= 0 ? 1.0 : r >= 1 ? n / (r - 1) : n * r / (r - 1);
}

// Find c for rand_expo<int>(0,n,c) so that freq(0)/freq(n)=r
double int_expo_base_for_ratio(int n, double r) {
    assert(r > 0);
    if (n <= 0) {
        return 1.0;
    }
    double t = max(r, 1 / r), L = 0, R = 1e5;
    for (int runs = 60; runs; runs--) {
        double c = (L + R) / 2;
        double f = log1p(1.0 / c) / log1p(1.0 / (c + n));
        f >= t ? L = c : R = c;
    }
    double c = (L + R) / 2;
    return r > 1 ? c : -c;
}

// Find p for rand_geom<real>(0,n,c) so that freq(0)/freq(n)=r
double real_geom_prob_for_ratio(double n, double r) {
    assert(r > 0);
    return n <= 0 ? 0.5 : r > 1 ? 1.0 - pow(r, -1.0 / n) : -(1.0 - pow(1 / r, -1.0 / n));
}

// Find p for rand_geom<int>(0,n,c) so that freq(0)/freq(n)=r
double int_geom_prob_for_ratio(int n, double r) {
    assert(r > 0);
    return n <= 0 ? 0.5 : r > 1 ? 1.0 - pow(r, -1.0 / n) : -(1.0 - pow(1 / r, -1.0 / n));
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_unif(int n, common_type_t<T> a, common_type_t<T> b) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_unif<T, O>(a, b);
    }
    return vec;
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_wide(int n, common_type_t<T> a, common_type_t<T> b, int draw) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_wide<T, O>(a, b, draw);
    }
    return vec;
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_grav(int n, common_type_t<T> a, common_type_t<T> b, int gravity) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_grav<T, O>(a, b, gravity);
    }
    return vec;
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_expo(int n, common_type_t<T> a, common_type_t<T> b, double c) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_expo<T, O>(a, b, c);
    }
    return vec;
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_geom(int n, common_type_t<T> a, common_type_t<T> b, double p) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_geom<T, O>(a, b, p);
    }
    return vec;
}

template <typename T, typename O = T> // ans=(u,v), u,v=[a,b] and u<=v
array<O, 2> ordered_unif(common_type_t<T> a, common_type_t<T> b) {
    static_assert(is_integral_v<T>);
    assert(b < numeric_limits<T>::max());
    auto x = rand_unif<T, O>(a, b);
    auto y = rand_unif<T, O>(a, b + 1);
    y -= y > x;
    return x <= y ? array<O, 2>{x, y} : array<O, 2>{y, x};
}

template <typename T, typename O = T> // ans=(u,v), u,v=[a,b] and u<v
array<O, 2> diff_unif(common_type_t<T> a, common_type_t<T> b) {
    static_assert(is_integral_v<T>);
    assert(a < b);
    auto x = rand_unif<T, O>(a, b);
    auto y = rand_unif<T, O>(a, b - 1);
    y += y >= x;
    return x < y ? array<O, 2>{x, y} : array<O, 2>{y, x};
}

template <typename T, typename O = T> // ans=[a,b] except banned
O diff_unif(common_type_t<T> banned, common_type_t<T> a, common_type_t<T> b) {
    static_assert(is_integral_v<T>);
    assert(a != banned ? a <= b : a < b);
    if (banned < a || b < banned) {
        return rand_unif<T, O>(a, b);
    } else {
        auto v = rand_unif<T, O>(a, b - 1);
        v += v >= banned;
        return v;
    }
}

auto rand_string(int len, char a, char b) {
    string str(len, '\0');
    for (int i = 0; i < len; i++) {
        str[i] = a + rand_unif<int>(0, b - a);
    }
    return str;
}

auto rand_strings(int n, int minlen, int maxlen, char a, char b) {
    vector<string> strs(n);
    for (int i = 0; i < n; i++) {
        strs[i] = rand_string(rand_unif<int>(minlen, maxlen), a, b);
    }
    return strs;
}

/**
 * Generate a sorted sample of k distinct integers from the range [a..b)
 * It must hold that a<=b and k<=m=b-a.
 */
template <typename T = int, typename I>
auto int_sample(int k, I a, I b) {
    using sample_t = vector<T>;
    if (k == 0 || a >= b) {
        return sample_t();
    }

    long long m = b - a;
    assert(k <= 100'000'000 && m > 0 && 1 <= k && k <= m);

    if (k == 1) {
        return sample_t{rand_unif<I>(a, b - 1)};
    }
    if (k == m) {
        sample_t sample(m);
        iota(begin(sample), end(sample), a);
        return sample;
    }
    if (k >= m / 2) {
        vector<bool> dead(m, false);
        int included = m;
        while (included > k) {
            int i = rand_unif<int>(0, m - 1);
            included -= !dead[i];
            dead[i] = true;
        }
        sample_t sample(k);
        for (int i = 0, j = 0; j < k; i++) {
            if (!dead[i]) {
                sample[j++] = a + i;
            }
        }
        return sample;
    }

    double harmonic = 0;
    for (int i = 0; i < k; i++) {
        harmonic += 1.0 / (m - i);
    }
    harmonic *= m;
    int size = ceil(harmonic);

    sample_t sample(size);
    for (int i = 0; i < size; i++) {
        sample[i] = rand_unif<I>(a, b - 1);
    }
    sort(begin(sample), end(sample));
    sample.erase(unique(begin(sample), end(sample)), end(sample));
    int s = sample.size();

    if (s > k) {
        set<int> dead;
        while (s - int(dead.size()) > k) {
            int i = rand_unif<int>(0, s - 1);
            dead.insert(i);
        }
        auto pos = begin(dead);
        sample_t output(k);
        for (int j = 0, i = 0; j < k; i++) {
            if (pos == end(dead) || *pos != i) {
                output[j++] = sample[i];
            } else {
                ++pos;
            }
        }
        swap(output, sample);
    } else if (s < k) {
        set<T> extra;
        while (s + int(extra.size()) < k) {
            auto nxt = rand_unif<I>(a, b - 1);
            if (!binary_search(begin(sample), end(sample), nxt)) {
                extra.insert(nxt);
            }
        }
        sample.insert(end(sample), begin(extra), end(extra));
        inplace_merge(begin(sample), begin(sample) + s, end(sample));
    }

    return sample;
}

template <typename T = int, typename I>
auto int_sample_p(double p, I a, I b) {
    long long m = b - a;
    if (m <= 100 || p > 0.20) {
        vector<T> choice;
        for (auto n = a; n < b; n++)
            if (cointoss(p))
                choice.push_back(n);
        return choice;
    }
    return int_sample<T, I>(binomd(m, p)(mt), a, b);
}

/**
 * Generate a sorted sample of k integer pairs (x,y) where x,y=[a..b) and x<y
 * It must hold that a<=b and k<=m=(b-a choose 2)
 */
template <typename T = int, typename I>
auto choose_sample(int k, I a, I b) {
    using sample_t = vector<array<T, 2>>;
    if (k == 0 || a >= b - 1) {
        return sample_t();
    }

    long long m = 1LL * (b - a) * (b - a - 1) / 2;
    assert(k <= 50'000'000 && m >= 1 && 1 <= k && k <= m);

    auto advance = [&](I& x, I& y) { y == b - 1 ? y = ++x + 1 : y++; };

    if (k == 1) {
        return sample_t{diff_unif<I, T>(a, b - 1)};
    }
    if (k == m) {
        sample_t sample(m);
        I x = a, y = a + 1;
        for (int i = 0; i < m; i++) {
            sample[i] = {x, y};
            advance(x, y);
        }
        return sample;
    }
    if (k >= m / 2) {
        vector<bool> dead(m, false);
        int included = m;
        while (included > k) {
            int i = rand_unif<int>(0, m - 1);
            included -= !dead[i];
            dead[i] = true;
        }
        sample_t sample(k);
        I x = a, y = a + 1;
        for (int j = 0, i = 0; j < k; i++) {
            if (!dead[i]) {
                sample[j++] = {x, y};
            }
            advance(x, y);
        }
        return sample;
    }

    double harmonic = 0;
    for (int i = 0; i < k; i++) {
        harmonic += 1.0 / (m - i);
    }
    harmonic *= m;
    int size = ceil(harmonic);

    sample_t sample(size);
    for (int i = 0; i < size; i++) {
        sample[i] = diff_unif<I, T>(a, b - 1);
    }
    sort(begin(sample), end(sample));
    sample.erase(unique(begin(sample), end(sample)), end(sample));
    int s = sample.size();

    if (s > k) {
        set<int> dead;
        while (s - int(dead.size()) > k) {
            int i = rand_unif<int>(0, s - 1);
            dead.insert(i);
        }
        auto pos = begin(dead);
        sample_t output(k);
        for (int j = 0, i = 0; j < k; i++) {
            if (pos == end(dead) || *pos != i) {
                output[j++] = sample[i];
            } else {
                ++pos;
            }
        }
        swap(output, sample);
    } else if (s < k) {
        set<array<T, 2>> extra;
        while (s + int(extra.size()) < k) {
            auto nxt = diff_unif<I, T>(a, b - 1);
            if (!binary_search(begin(sample), end(sample), nxt)) {
                extra.insert(nxt);
            }
        }
        sample.insert(end(sample), begin(extra), end(extra));
        inplace_merge(begin(sample), begin(sample) + s, end(sample));
    }

    return sample;
}

template <typename T = int, typename I>
auto choose_sample_p(double p, I a, I b) {
    long long m = 1LL * (b - a) * (b - a - 1) / 2;
    if (m <= 100 || p > 0.20) {
        vector<array<T, 2>> choice;
        for (auto x = a; x < b; x++)
            for (auto y = x + 1; y < b; y++)
                if (cointoss(p))
                    choice.push_back({x, y});
        return choice;
    }
    return choose_sample<T, I>(binomd(m, p)(mt), a, b);
}

/**
 * Generate a sorted sample of k integer pairs (x,y) where x=[a..b) and y=[c..d)
 * It must hold that a<=b, c<=d, and k<=nm=(b-a)(d-c).
 */
template <typename T = int, typename I>
auto pair_sample(int k, I a, I b, I c, I d) {
    using sample_t = vector<array<T, 2>>;
    if (k == 0 || a >= b || c >= d)
        return sample_t();

    long m = 1LL * (b - a) * (d - c);
    assert(k <= 50'000'000 && m >= 1 && 1 <= k && k <= m);

    if (k == 1) {
        auto x = rand_unif<I, T>(a, b - 1);
        auto y = rand_unif<I, T>(c, d - 1);
        return sample_t{{x, y}};
    }
    if (k == m) {
        sample_t whole(m);
        int i = 0;
        for (I x = a; x < b; x++) {
            for (I y = c; y < d; y++) {
                whole[i++] = {x, y};
            }
        }
        return whole;
    }
    if (k >= m / 2) {
        vector<bool> dead(m, false);
        int included = m;
        while (included > k) {
            int i = rand_unif<int>(0, m - 1);
            included -= !dead[i];
            dead[i] = true;
        }
        sample_t sample(k);
        I x = a, y = c;
        for (int j = 0, i = 0; j < k; i++) {
            if (!dead[i]) {
                sample[j++] = {x, y};
            }
            y == d - 1 ? x++, y = c : y++;
        }
        return sample;
    }

    double harmonic = 0;
    for (int i = 0; i < k; i++) {
        harmonic += 1.0 / (m - i);
    }
    harmonic *= m;
    int size = ceil(harmonic);

    sample_t sample(size);
    for (int i = 0; i < size; i++) {
        auto x = rand_unif<I, T>(a, b - 1);
        auto y = rand_unif<I, T>(c, d - 1);
        sample[i] = {x, y};
    }
    sort(begin(sample), end(sample));
    sample.erase(unique(begin(sample), end(sample)), end(sample));
    int s = sample.size();

    if (s > k) {
        set<int> dead;
        while (s - int(dead.size()) > k) {
            int i = rand_unif<int>(0, s - 1);
            dead.insert(i);
        }
        auto pos = begin(dead);
        sample_t output(k);
        for (int j = 0, i = 0; j < k; i++) {
            if (pos == end(dead) || *pos != i) {
                output[j++] = sample[i];
            } else {
                ++pos;
            }
        }
        swap(output, sample);
    } else if (s < k) {
        set<array<T, 2>> extra;
        while (s + int(extra.size()) < k) {
            auto x = rand_unif<I, T>(a, b - 1);
            auto y = rand_unif<I, T>(c, d - 1);
            array<T, 2> nxt = {x, y};
            if (!binary_search(begin(sample), end(sample), nxt)) {
                extra.insert(nxt);
            }
        }
        sample.insert(end(sample), begin(extra), end(extra));
        inplace_merge(begin(sample), begin(sample) + s, end(sample));
    }

    return sample;
}

template <typename T = int, typename I>
auto pair_sample_p(double p, I a, I b, I c, I d) {
    long long m = 1LL * (b - a) * (d - c);
    if (m <= 100 || p > 0.20) {
        boold coind(p);
        vector<array<T, 2>> choice;
        for (auto x = a; x < b; x++)
            for (auto y = c; y < d; y++)
                if (coind(mt))
                    choice.push_back({x, y});
        return choice;
    }
    return pair_sample<T, I>(binomd(m, p)(mt), a, b, c, d);
}

/**
 * Generate an unsorted sample of k integers pairs (x,y) where x,y=[a..b) and x!=y.
 * It must hold that a<=b, and k<=(b-a)(b-a-1).
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
    long long m = 1LL * (b - a) * (b - a - 1);
    if (m <= 100 || p > 0.20) {
        boold coind(p);
        vector<array<T, 2>> choice;
        for (auto x = a; x < b; x++)
            for (auto y = a; y < b; y++)
                if (x != y && coind(mt))
                    choice.push_back({x, y});
        return choice;
    }
    return distinct_pair_sample(binomd(m, p)(mt), a, b);
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
 */
template <typename I = int>
auto partition_sample(I n, int k, I m, I M = numeric_limits<I>::max()) {
    if (n == 0) {
        return vector<I>{};
    }
    assert(n >= 0 && k > 0 && m >= 0 && m <= n / k && (n + k - 1) / k <= M);

    if (M >= n) {
        vector<I> parts(k, m);
        n -= m * k;
        auto cuts = int_sample<I>(k - 1, 1, n + k);
        cuts.insert(begin(cuts), 0), cuts.push_back(n + k);
        for (int i = 0; i < k; i++) {
            parts[i] += cuts[i + 1] - cuts[i] - 1;
        }
        return parts;
    } else {
        vector<I> parts(k, m);
        n -= m * k--;
        while (n > 0) {
            I add = (n + k) / (k + 1);
            int i = rand_unif<int>(0, k);
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
}

/**
 * Generate a partition of n into k parts each of size between m[i] and M[i].
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
        int i = rand_unif<int>(0, k), j = id[i];
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
auto partition_sample_flow(int V, int ranks, int m, int M = numeric_limits<int>::max()) {
    auto R = partition_sample(V - 2, ranks - 2, m, M);
    R.insert(R.begin(), 1);
    R.insert(R.end(), 1);
    return R;
}

/**
 * Like partition_sample but the first and last levels have size exactly 1.
 */
auto partition_sample_flow(int V, int ranks, const vector<int>& m, const vector<int>& M) {
    auto R = partition_sample(V - 2, ranks - 2, m, M);
    R.insert(R.begin(), 1);
    R.insert(R.end(), 1);
    return R;
}

/**
 * Generate a supply partition with n elements whose total sum is c and minimum is m.
 */
template <typename I = int>
auto supply_sample(int n, int positives, int negatives, I sum, I m) {
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
