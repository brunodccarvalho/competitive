#pragma once

#include "hash.hpp"
#include "algo/sort.hpp"
#include "struct/pbds.hpp"

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

template <typename T> // ans=[a,b], uniform
T rand_unif(common_type_t<T> a, common_type_t<T> b) {
    assert(a <= b);
    if constexpr (is_integral_v<T>) {
        return uniform_int_distribution<T>(T(a), T(b))(mt);
    } else if constexpr (is_floating_point_v<T>) {
        return uniform_real_distribution<T>(T(a), T(b))(mt);
    } else {
        assert(false);
    }
}

template <typename T> // ans=[a,b], uniform, <0=>min, >0=>max
T rand_wide(common_type_t<T> a, common_type_t<T> b, int draw) {
    assert(-20 <= draw && draw <= 20);
    auto ans = rand_unif<T>(a, b);
    while (draw > 0)
        ans = max(ans, rand_unif<T>(a, b)), draw--;
    while (draw < 0)
        ans = min(ans, rand_unif<T>(a, b)), draw++;
    return ans;
}

template <typename T> // ans=[a,b], uniform, <0=>vee, >0=>normal
T rand_grav(common_type_t<T> a, common_type_t<T> b, int grav) {
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

template <typename T> // ans=[a,b], exponential, slope 1/(c+i), c!=0
T rand_expo(common_type_t<T> a, common_type_t<T> b, double c) {
    if (a >= b) {
        return a;
    } else if (c < 0) {
        return b - rand_expo<T>(a, b, -c) + a;
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

template <typename T> // ans=[a,b], geometric, slope 1-p, -1<p<1, p!=0
T rand_geom(common_type_t<T> a, common_type_t<T> b, double p) {
    if (a >= b) {
        return a;
    } else if (p < 0.0) {
        return b - rand_geom<T>(a, b, -p) + a;
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

template <typename T> // ans=[a,b], normal, round towards nearest
T rand_norm(common_type_t<T> a, common_type_t<T> b, double mean, double dev) {
    if constexpr (is_integral_v<T>) {
        assert(.25 * dev <= (b - a + 1) && a - dev <= mean && mean <= b + dev);
        long long x;
        normal_distribution dist(mean, dev);
        do {
            x = llround(dist(mt));
        } while (x < a || b < x);
        return x;
    } else if constexpr (is_floating_point_v<T>) {
        assert(.25 * dev <= (b - a) && a - dev <= mean && mean <= b + dev);
        double x;
        normal_distribution dist(mean, dev);
        do {
            x = dist(mt);
        } while (x < a || b < x);
        return x;
    } else {
        assert(false);
    }
}

template <typename T> // ans=[a,b], gravitate towards peak
T rand_peak(common_type_t<T> a, common_type_t<T> b, T peak, int grav) {
    assert(-20 <= grav && grav <= 20);
    auto ans = rand_unif<T>(a, b);
    while (grav > 0) {
        auto nxt = rand_unif<T>(a, b);
        ans = abs(ans - peak) <= abs(nxt - peak) ? ans : nxt, grav--;
    }
    while (grav < 0) {
        auto nxt = rand_unif<T>(a, b);
        ans = abs(ans - peak) >= abs(nxt - peak) ? ans : nxt, grav++;
    }
    return ans;
}

// Find c for rand_expo<real>(0,n,c) so that freq(0)/freq(n)=r
// To get freq(b)/freq(n)=n/b when n goes to infinity pick c=b
double real_expo_base_for_ratio(double n, double r) {
    assert(r > 0);
    return n <= 0 ? 1.0 : r > 1 ? n / (r - 1) : n * r / (r - 1);
}

// Find p for rand_geom<real>(0,n,c) so that freq(0)/freq(n)=r
double real_geom_prob_for_ratio(double n, double r) {
    assert(r > 0);
    return n <= 0 ? 0.5 : r > 1 ? 1.0 - pow(r, -1.0 / n) : -(1.0 - pow(1 / r, -1.0 / n));
}

// Find c for rand_expo<int>(b,n,c) so that freq(b)/freq(n)=n/b when n goes to infinity
double int_expo_base_for_ratio(double b) {
    assert(b > 0);
    return 1 / expm1(1.0 / b);
}

// Find p for rand_geom<int>(0,n,c) so that freq(0)/freq(n)=r
double int_geom_prob_for_ratio(int64_t n, double r) {
    assert(r > 0);
    return n <= 0 ? 0.5 : r > 1 ? 1.0 - pow(r, -1.0 / n) : -(1.0 - pow(1 / r, -1.0 / n));
}

// Find c for rand_expo<int>(0,n,c) so that freq(0)/freq(n)=r
double int_expo_base_for_ratio(int64_t n, double r) {
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

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_unif(int n, common_type_t<T> a, common_type_t<T> b) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_unif<T>(a, b);
    }
    return vec;
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_wide(int n, common_type_t<T> a, common_type_t<T> b, int draw) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_wide<T>(a, b, draw);
    }
    return vec;
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_grav(int n, common_type_t<T> a, common_type_t<T> b, int gravity) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_grav<T>(a, b, gravity);
    }
    return vec;
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_expo(int n, common_type_t<T> a, common_type_t<T> b, double c) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_expo<T>(a, b, c);
    }
    return vec;
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_geom(int n, common_type_t<T> a, common_type_t<T> b, double p) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_geom<T>(a, b, p);
    }
    return vec;
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_norm(int n, common_type_t<T> a, common_type_t<T> b, double mean,
                     double dev) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_norm<T>(a, b, mean, dev);
    }
    return vec;
}

template <typename T, typename O = T> // ans=[a,b]
vector<O> rands_peak(int n, common_type_t<T> a, common_type_t<T> b, T peak, int grav) {
    vector<O> vec(n);
    for (int i = 0; i < n; i++) {
        vec[i] = rand_peak<T>(a, b, peak, grav);
    }
    return vec;
}

template <typename T, typename O = T> // ans=(u,v), u,v=[a,b] and u<=v
array<O, 2> ordered_unif(common_type_t<T> a, common_type_t<T> b) {
    if constexpr (is_integral_v<T>) {
        assert(b < numeric_limits<T>::max());
        auto x = rand_unif<T>(a, b);
        auto y = rand_unif<T>(a, b + 1);
        y -= y > x;
        return x <= y ? array<O, 2>{x, y} : array<O, 2>{y, x};
    } else if constexpr (is_floating_point_v<T>) {
        auto x = rand_unif<T>(a, b);
        auto y = rand_unif<T>(a, b);
        return x <= y ? array<O, 2>{x, y} : array<O, 2>{y, x};
    } else {
        assert(false);
    }
}

template <typename T, typename O = T> // ans=(u,v), u,v=[a,b] and u<v
array<O, 2> diff_unif(common_type_t<T> a, common_type_t<T> b) {
    static_assert(is_integral_v<T>);
    assert(a < b);
    auto x = rand_unif<T>(a, b);
    auto y = rand_unif<T>(a, b - 1);
    y += y >= x;
    return x < y ? array<O, 2>{x, y} : array<O, 2>{y, x};
}

template <typename T, typename O = T> // ans=[a,b] except banned
O diff_unif(common_type_t<T> banned, common_type_t<T> a, common_type_t<T> b) {
    static_assert(is_integral_v<T>);
    assert(a != banned ? a <= b : a < b);
    if (banned < a || b < banned) {
        return rand_unif<T>(a, b);
    } else {
        auto v = rand_unif<T>(a, b - 1);
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
template <typename T>
auto int_sample(int k, T a, T b) {
    using sample_t = vector<T>;
    if (k == 0 || a >= b) {
        return sample_t();
    }

    long long m = b - a;
    assert(k <= 100'000'000 && a < b && 1 <= k && k <= m);

    if (k == 1) {
        return sample_t{rand_unif<T>(a, b - 1)};
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
        for (int i = 0, j = 0; i < m && j < k; i++) {
            if (!dead[i]) {
                sample[j++] = a + i;
            }
        }
        return sample;
    }

    double harmonic = m * log1p(1.0 * k / (m - k));
    int size = max(k, int(llround(harmonic)));

    sample_t sample(size);
    for (int i = 0; i < size; i++) {
        sample[i] = rand_unif<T>(a, b - 1);
    }
    lsb_radix_sort(sample);
    sample.erase(unique(begin(sample), end(sample)), end(sample));
    int s = sample.size();

    if (s > k) {
        set<T> dead;
        while (s - int(dead.size()) > k) {
            int i = rand_unif<int>(0, s - 1);
            dead.insert(sample[i]);
        }
        set_difference(begin(sample), end(sample), begin(dead), end(dead), begin(sample));
        sample.resize(k);
    } else if (s < k) {
        set<T> extra;
        while (s + int(extra.size()) < k) {
            auto nxt = rand_unif<T>(a, b - 1);
            if (!binary_search(begin(sample), end(sample), nxt)) {
                extra.insert(nxt);
            }
        }
        sample.insert(end(sample), begin(extra), end(extra));
        inplace_merge(begin(sample), begin(sample) + s, end(sample));
    }

    return sample;
}

template <typename T>
auto int_sample_p(double p, T a, T b) {
    long long m = b - a;
    if (m <= 100 || p > 0.20) {
        vector<T> choice;
        for (auto n = a; n < b; n++)
            if (cointoss(p))
                choice.push_back(n);
        return choice;
    }
    return int_sample<T>(binomd(m, p)(mt), a, b);
}

/**
 * Generate a sorted sample of k integer pairs (x,y) where x,y=[a..b) and x<y
 * It must hold that a<=b and k<=m=(b-a choose 2)
 */
template <typename T>
auto choose_sample(int k, T a, T b) {
    using sample_t = vector<array<T, 2>>;
    if (k == 0 || a >= b - 1) {
        return sample_t();
    }

    long long m = 1LL * (b - a) * (b - a - 1) / 2;
    assert(k <= 50'000'000 && a < b && 1 <= k && k <= m);

    if (k == 1) {
        return sample_t{diff_unif<T>(a, b - 1)};
    }
    if (k == m) {
        sample_t sample(m);
        T x = a, y = a + 1;
        for (int i = 0; i < m; i++) {
            sample[i] = {x, y};
            y == b - 1 ? y = ++x + 1 : y++;
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
        T x = a, y = a + 1;
        for (int j = 0, i = 0; i < m && j < k; i++) {
            if (!dead[i]) {
                sample[j++] = {x, y};
            }
            y == b - 1 ? y = ++x + 1 : y++;
        }
        return sample;
    }

    double harmonic = m * log1p(1.0 * k / (m - k));
    int size = max(k, int(llround(harmonic)));

    sample_t sample(size);
    for (int i = 0; i < size; i++) {
        sample[i] = diff_unif<T>(a, b - 1);
    }
    sort(begin(sample), end(sample));
    sample.erase(unique(begin(sample), end(sample)), end(sample));
    int s = sample.size();

    if (s > k) {
        set<array<T, 2>> dead;
        while (s - int(dead.size()) > k) {
            int i = rand_unif<int>(0, s - 1);
            dead.insert(sample[i]);
        }
        set_difference(begin(sample), end(sample), begin(dead), end(dead), begin(sample));
        sample.resize(k);
    } else if (s < k) {
        set<array<T, 2>> extra;
        while (s + int(extra.size()) < k) {
            auto nxt = diff_unif<T>(a, b - 1);
            if (!binary_search(begin(sample), end(sample), nxt)) {
                extra.insert(nxt);
            }
        }
        sample.insert(end(sample), begin(extra), end(extra));
        inplace_merge(begin(sample), begin(sample) + s, end(sample));
    }

    return sample;
}

template <typename T>
auto choose_sample_p(double p, T a, T b) {
    long long m = 1LL * (b - a) * (b - a - 1) / 2;
    if (m <= 100 || p > 0.20) {
        vector<array<T, 2>> choice;
        for (auto x = a; x < b; x++)
            for (auto y = x + 1; y < b; y++)
                if (cointoss(p))
                    choice.push_back({x, y});
        return choice;
    }
    return choose_sample<T>(binomd(m, p)(mt), a, b);
}

/**
 * Generate a sorted sample of k integer pairs (x,y) where x=[a..b) and y=[c..d)
 * It must hold that a<=b, c<=d, and k<=nm=(b-a)(d-c).
 */
template <typename T>
auto pair_sample(int k, T a, T b, T c, T d) {
    using sample_t = vector<array<T, 2>>;
    if (k == 0 || a >= b || c >= d)
        return sample_t();

    long m = 1LL * (b - a) * (d - c);
    assert(k <= 50'000'000 && a < b && c < d && m >= 1 && 1 <= k && k <= m);

    if (k == 1) {
        auto x = rand_unif<T>(a, b - 1);
        auto y = rand_unif<T>(c, d - 1);
        return sample_t{{x, y}};
    }
    if (k == m) {
        sample_t whole(m);
        int i = 0;
        for (T x = a; x < b; x++) {
            for (T y = c; y < d; y++) {
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
        T x = a, y = c;
        for (int j = 0, i = 0; j < k; i++) {
            if (!dead[i]) {
                sample[j++] = {x, y};
            }
            y == d - 1 ? x++, y = c : y++;
        }
        return sample;
    }

    double harmonic = m * log1p(1.0 * k / (m - k));
    int size = max(k, int(llround(harmonic)));

    sample_t sample(size);
    for (int i = 0; i < size; i++) {
        auto x = rand_unif<T>(a, b - 1);
        auto y = rand_unif<T>(c, d - 1);
        sample[i] = {x, y};
    }
    sort(begin(sample), end(sample));
    sample.erase(unique(begin(sample), end(sample)), end(sample));
    int s = sample.size();

    if (s > k) {
        set<array<T, 2>> dead;
        while (s - int(dead.size()) > k) {
            int i = rand_unif<int>(0, s - 1);
            dead.insert(sample[i]);
        }
        set_difference(begin(sample), end(sample), begin(dead), end(dead), begin(sample));
        sample.resize(k);
    } else if (s < k) {
        set<array<T, 2>> extra;
        while (s + int(extra.size()) < k) {
            auto x = rand_unif<T>(a, b - 1);
            auto y = rand_unif<T>(c, d - 1);
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

template <typename T>
auto pair_sample_p(double p, T a, T b, T c, T d) {
    long long m = 1LL * (b - a) * (d - c);
    if (m <= 100 || p > 0.20) {
        vector<array<T, 2>> choice;
        for (auto x = a; x < b; x++)
            for (auto y = c; y < d; y++)
                if (cointoss(p))
                    choice.push_back({x, y});
        return choice;
    }
    return pair_sample<T>(binomd(m, p)(mt), a, b, c, d);
}

/**
 * Generate an unsorted sample of k integers pairs (x,y) where x,y=[a..b) and x!=y.
 * It must hold that a<=b, and k<=(b-a)(b-a-1).
 */
template <typename T>
auto distinct_pair_sample(int k, T a, T b) {
    auto g = pair_sample(k, a, b, a, b - 1);
    for (auto& [u, v] : g)
        v += v >= u;
    return g;
}

template <typename T>
auto distinct_pair_sample_p(double p, T a, T b) {
    long long m = 1LL * (b - a) * (b - a - 1);
    if (m <= 100 || p > 0.20) {
        vector<array<T, 2>> choice;
        for (auto x = a; x < b; x++)
            for (auto y = a; y < b; y++)
                if (x != y && cointoss(p))
                    choice.push_back({x, y});
        return choice;
    }
    return distinct_pair_sample<T>(binomd(m, p)(mt), a, b);
}

/**
 * Generate a sorted sample of k distinct integers from [a..b) with given generator gen(n)
 * which outputs integers in range [1,n]
 */
template <typename T, typename Fn>
auto compound_sample(int k, T a, T b, Fn&& gen) {
    static_assert(is_integral_v<T>);
    long long u = b - a;
    ordered_set<T> sample;

    for (int i = 0; i < k; i++) {
        auto g = gen(u - i);
        T l = a, r = b;
        while (l + 1 < r) {
            T m = (l + r) / 2;
            T open = (m - a) - sample.order_of_key(m);
            open >= g ? r = m : l = m;
        }
        sample.insert(l);
    }
    return vector<T>(begin(sample), end(sample));
}

template <typename T>
auto geom_sample(int k, T a, T b, double p) {
    return compound_sample(k, a, b, [p](T n) { //
        return rand_geom<T>(1, n, p);
    });
}

template <typename T>
auto norm_sample(int k, T a, T b, double norm, double dev) {
    double B = (norm - a + 0.5) / (b - a), G = dev / (b - a);
    return compound_sample(k, a, b, [B, G](T n) { //
        return rand_norm<T>(1, n, .5 + B * n, G * n);
    });
}

/**
 * Generate a partition not exceeding sum by sampling gen(sum) a bunch of times
 */
template <typename T, typename Fn>
auto rand_partial_partition(T sum, Fn&& gen) {
    static_assert(is_integral_v<T>);
    vector<T> parts;
    int skips = 10;
    while (skips > 0 && sum > 0) {
        T v = gen(sum);
        if (sum >= v) {
            parts.push_back(v);
            sum -= v;
        } else {
            skips--;
        }
    }
    return parts;
}

/**
 * Generate a partition of exactly sum by sampling from an exponential distribution
 */
template <typename T>
auto rand_expo_partition(T sum, T maximum) {
    static_assert(is_integral_v<T>);
    const double c = int_expo_base_for_ratio(1); // 0.581977
    return rand_partial_partition(sum, [&](T cur) {
        return rand_expo<T>(1, min(cur, maximum), c);
    });
}

/**
 * Generate a random partition of n into k parts each of size between m and M.
 */
template <typename T>
auto partition_sample(T n, int k, T m, T M = numeric_limits<T>::max()) {
    if (n == 0) {
        return vector<T>{};
    }
    assert(n >= 0 && k > 0 && m >= 0 && m <= n / k && (n + k - 1) / k <= M);

    if (M >= n) {
        vector<T> parts(k, m);
        n -= m * k;
        auto cuts = int_sample<T>(k - 1, 1, n + k);
        cuts.insert(begin(cuts), 0), cuts.push_back(n + k);
        for (int i = 0; i < k; i++) {
            parts[i] += cuts[i + 1] - cuts[i] - 1;
        }
        return parts;
    } else {
        vector<T> parts(k, m);
        n -= m * k--;
        while (n > 0) {
            T add = (n + k) / (k + 1);
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
template <typename T>
auto partition_sample(T n, int k, const vector<T>& m, const vector<T>& M) {
    assert(k > 0 && k <= int(m.size()) && k <= int(M.size()));

    vector<T> parts(k);
    vector<int> id(k);
    copy(begin(m), begin(m) + k--, begin(parts));
    iota(begin(id), end(id), 0);
    n -= accumulate(begin(parts), end(parts), T(0));
    while (n > 0) {
        T add = (n + k) / (k + 1);
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
