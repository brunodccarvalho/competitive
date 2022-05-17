#pragma once

#include <bits/stdc++.h>
using namespace std;

constexpr int64_t intpow(int64_t b, int64_t e) {
    int64_t power = 1;
    while (e > 0) {
        if (e & 1) {
            power = power * b;
        }
        e >>= 1;
        b = b * b;
    }
    return power;
}

constexpr int64_t modpow(int64_t base, int64_t e, int64_t mod) {
    int64_t x = 1;
    base = base % mod;
    while (e > 0) {
        if (e & 1)
            x = __int128_t(x) * __int128_t(base) % mod;
        if (e >>= 1)
            base = __int128_t(base) * __int128_t(base) % mod;
    }
    return x;
}

constexpr int modpow(int base, int64_t e, int mod) {
    int x = 1;
    base = base % mod;
    while (e > 0) {
        if (e & 1)
            x = int64_t(x) * int64_t(base) % mod;
        if (e >>= 1)
            base = int64_t(base) * int64_t(base) % mod;
    }
    return x;
}

long intfac(int n) {
    long f = 1;
    while (n > 1) {
        f = f * n--;
    }
    return f;
}

long modfac(int n, long m) {
    long f = 1;
    while (n > 1) {
        f = (f * n--) % m;
    }
    return f;
}

long lcm(long a, long b) { return a * (b / gcd(a, b)); }

long gcd(long a, long b) {
    while (a != 0) {
        b = b % a;
        swap(a, b);
    }
    return abs(b);
}

long gcd(long a, long b, long& x, long& y) {
    int64_t xn = 1, yn = 0;
    x = 0, y = 1;
    while (a != 0) {
        int64_t q = b / a;
        b = b % a;
        x = x - q * xn, y = y - q * yn;
        swap(a, b), swap(x, xn), swap(y, yn);
    }
    if (b < 0) {
        b = -b, x = -x, y = -y;
    }
    return b;
}

// Compute x such that ax = 1 (mod m) (modular multiplicative inverse)
long invmod(long a, long m) {
    long x, y, g = gcd(a, m, x, y);
    (void)g, assert(g == 1);
    x = x % m;
    return x < 0 ? x + m : x;
}

/**
 * Compute the smallest exponent x such that a^x = b (mod m)
 * Complexity: O(sqrt(m)), uses square root decomposition
 * Source: kth (https://github.com/kth-competitive-programming/kactl)
 */
long modlog(long a, long b, long m) {
    long n = long(sqrt(m)) + 1, x = 1, f = 1, j = 1;
    unordered_map<long, long> A;
    while (j <= n && (x = f = x * a % m) != b % m)
        A[x * b % m] = j++;
    if (x == b % m)
        return j;
    if (gcd(m, x) == gcd(m, b))
        for (int i = 2; i < n + 2; i++)
            if (A.count(x = x * f % m))
                return n * i - A[x];
    return -1;
}

/**
 * Compute x such that x^2 = a (mod p) where p is prime
 * Complexity: O(log^2 p) heavy
 * Source: kth (https://github.com/kth-competitive-programming/kactl)
 */
long modsqrt(long a, long p) {
    a = a % p, a = a >= 0 ? a : a + p;
    if (a == 0)
        return 0;
    if (modpow(a, (p - 1) / 2, p) != 1)
        return -1; // not a quadratic residue
    if (p % 4 == 3)
        return modpow(a, (p + 1) / 4, p);
    // a^(n+3)/8 or 2^(n+3)/8 * 2^(n-1)/4 works if p % 8 == 5
    long s = p - 1, n = 2;
    int r = 0, m;
    while (s % 2 == 0)
        ++r, s /= 2;
    /// find a non-square mod p
    while (modpow(n, (p - 1) / 2, p) != p - 1)
        ++n;
    long x = modpow(a, (s + 1) / 2, p);
    long b = modpow(a, s, p), g = modpow(n, s, p);
    for (;; r = m) {
        long t = b;
        for (m = 0; m < r && t != 1; ++m)
            t = t * t % p;
        if (m == 0)
            return x;
        long gs = modpow(g, 1L << (r - m - 1), p);
        g = gs * gs % p;
        x = x * gs % p;
        b = b * g % p;
    }
}

// Solve the system a = b[i] (mod p[i]), i = 0,...,n-1. Complexity: O(n log p)
long chinese(int n, long* b, long* primes) {
    long p = 1, a = 0;
    for (int i = 0; i < n; i++) {
        long q = primes[i], x, y, g = gcd(p, q, x, y);
        (void)g, assert(g == 1);
        a = a * q * y + b[i] * p * x;
        p = p * q;
        a = a % p, a = a >= 0 ? a : a + p;
    }
    return a;
}

// Compute phi(n) (totient function), naively
long phi(long n) {
    long tot = 1;
    if ((n & 1) == 0) {
        n >>= 1;
        while ((n & 1) == 0)
            tot <<= 1, n >>= 1;
    }
    for (long p = 3; p * p <= n; p += 2) {
        if (n % p == 0) {
            tot *= p - 1;
            n = n / p;
            while (n % p == 0) {
                tot *= p;
                n = n / p;
            }
        }
    }
    tot *= n > 1 ? n - 1 : 1;
    return tot;
}

auto get_primes(int n) {
    map<int, int> primes;
    for (int p = 2; p * p <= n; p++) {
        while (n % p == 0) {
            n /= p;
            primes[p]++;
        }
    }
    if (n > 1) {
        primes[n]++;
    }
    return primes;
}

template <typename T>
auto get_divisors(const map<T, int>& factors, bool one, bool self) {
    vector<T> divs = {1};
    for (const auto& [p, e] : factors) {
        int D = divs.size();
        divs.resize(D * (e + 1));
        for (int n = 1; n <= e; n++) {
            for (int d = 0; d < D; d++) {
                divs[d + n * D] = divs[d + (n - 1) * D] * p;
            }
        }
    }
    if (!one) {
        divs.erase(begin(divs));
    }
    if (!self && !divs.empty()) {
        divs.erase(begin(divs) + (divs.size() - 1));
    }
    return divs;
}

/**
 * Compute the jacobi (a/b) = ±1 (from quadratic reciprocity)
 * Complexity: O(log ab)
 */
int jacobi(long a, long b) {
    assert(b & 1);
    int t = 1, r;
    a = a % b;
    while (a != 0) {
        while ((a & 1) == 0) {
            a >>= 1, r = b % 8;
            if (r == 3 || r == 5)
                t = -t;
        }
        swap(a, b);
        if ((a % 4) == 3 && (b % 4) == 3)
            t = -t;
        a = a % b;
    }
    return b == 1 ? t : 0;
}

/**
 * Determines if a is a quadratic residue mod p (p prime)
 * Complexity: O(log p)
 */
bool is_quadratic_residue(long a, long p) {
    a = a % p, a = a >= 0 ? a : a + p;
    return a != 0 && modpow(a, (p - 1) / 2, p) == 1;
}

/**
 * Fast miller-rabin test for compositeness of n
 * Returns true if n is a prime.
 * Complexity: O(log n)
 */
constexpr bool miller_rabin(int64_t n) {
    if (n < 5)
        return n == 2 || n == 3;
    if (n % 2 == 0)
        return false;
    int r = __builtin_ctzll(n - 1);
    int64_t d = n >> r;

    for (int witness : {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37}) {
        if (witness > n)
            break;
        if (witness == n)
            return true;
        auto x = modpow(int64_t(witness), d, n);
        if (x == 1 || x == n - 1)
            continue;
        for (int i = 0; i < r - 1 && x != n - 1; i++) {
            x = __int128_t(x) * __int128_t(x) % n;
        }
        if (x != n - 1)
            return false;
    }
    return true;
}

// Compute first primitive root modulo prime p. Complexity: O(sqrt(p)) worst-case
constexpr int primitive_root_prime(int p) {
    if (p == 2 || p == 3)
        return p - 1;
    if (p == 167772161 || p == 469762049 || p == 998244353)
        return 3;
    if (p == 1000000007)
        return 5;
    assert(p % 2 == 1);

    int divs[20] = {2};
    int cnt = 1;
    int x = (p - 1) / 2; // phi(p)
    while (x % 2 == 0) {
        x /= 2;
    }
    for (int i = 3; i * i <= x; i += 2) {
        if (x % i == 0) {
            divs[cnt++] = i;
            while (x % i == 0) {
                x /= i;
            }
        }
    }
    if (x > 1) {
        divs[cnt++] = x;
    }
    for (int g = 2;; g++) {
        bool ok = true;
        for (int i = 0; i < cnt; i++) {
            if (modpow(g, (p - 1) / divs[i], p) == 1) {
                ok = false;
                break;
            }
        }
        if (ok) {
            return g;
        }
    }
    __builtin_unreachable();
}

// Compute (n choose k)
long choose(int n, int k) {
    if (k < 0 || k > n)
        return 0;
    k = min(k, n - k);
    n = n - k + 1;
    long binom = 1;
    int i = 1;
    while (i <= k) {
        binom = (binom * n++) / i++;
    }
    return binom;
}

// Compute n!/(k1!k2!k3!...) (multinomial)
long choose(long n, const vector<int>& ks) {
    assert(accumulate(begin(ks), end(ks), 0L) <= n);
    long multi = 1;
    int m = 1, r = 1;
    for (int i = 0, K = ks.size(); i < K; i++) {
        for (int j = 1; j <= ks[i]; j++) {
            multi = multi * m++ / j;
        }
    }
    while (m < n) {
        multi = multi * m++ / r++;
    }
    return multi;
}

// Compute (n choose k) (mod m)
long choosemod(long n, long k, long m) {
    if (k < 0 || k > n)
        return 0;
    long x = modfac(n, m);
    x = x * invmod(modfac(n - k, m), m) % m;
    x = x * invmod(modfac(k, m), m) % m;
    return x;
}

// Compute n!/(k1!k2!k3!...) (mod m) (multinomial)
long choosemod(long n, vector<int>& ks, long m) {
    assert(accumulate(begin(ks), end(ks), 0) == n);
    long x = modfac(n, m);
    for (int k : ks) {
        x = x * invmod(modfac(k, m), m) % m;
    }
    return x;
}

// Number of partitions of an integer n.
long partitions(long n) {
    static int m = 5;
    static long table[121] = {1, 1, 2, 3, 5, 7};
    assert(n <= 120);
    while (m < n) {
        m++;
        for (int k = 1, t = 1; k * (3 * k - 1) / 2 <= m; k++, t = -t)
            table[m] += t * table[m - k * (3 * k - 1) / 2];
        for (int k = -1, t = 1; k * (3 * k - 1) / 2 <= m; k--, t = -t)
            table[m] += t * table[m - k * (3 * k - 1) / 2];
    }
    return table[n];
}

// Probability of k successes in a binomial(n,p)
template <typename D = double>
D binom_success(int n, int k, D p, const vector<D>& logfac) {
    return exp(logfac[n] - logfac[k] - logfac[n - k] + k * log(p) + (n - k) * log1p(-p));
}

template <typename Vec, typename D = typename Vec::value_type>
D kahan_sum(const Vec& doubles) {
    D sum = 0, c = 0;
    for (D num : doubles) {
        D y = num - c;
        D t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}

template <typename D, typename Add = D>
void kahan_add(D& sum, D& compensation, Add num) {
    D y = num - compensation;
    D t = sum + y;
    compensation = (t - sum) - y;
    sum = t;
}
