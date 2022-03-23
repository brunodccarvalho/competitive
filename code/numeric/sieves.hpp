#pragma once

#include <bits/stdc++.h>
using namespace std;

// Compute all primes p<=N. This allows querying primes or number of primes in a range
// [L,R] at most N*N. O(N log log N)
auto classic_sieve(int N) {
    vector<int> primes;
    vector<bool> isprime;
    isprime.assign(N + 1, true);

    for (int n = 4; n <= N; n += 2)
        isprime[n] = false;
    for (int n = 3; n * n <= N; n += 2)
        if (isprime[n])
            for (int i = n * n; i <= N; i += 2 * n)
                isprime[i] = false;
    for (int n = 2; n <= N; n++)
        if (isprime[n])
            primes.push_back(n);

    return primes;
}

// Compute least prime divisors and n/lp[n] of all n<=N. O(N)
auto least_prime_sieve(int N) {
    vector<int> primes, lp, nxt;
    lp.assign(N + 1, 0), nxt.assign(N + 1, 0);
    nxt[1] = 1;

    for (int P = 0, n = 2; n <= N; n++) {
        if (lp[n] == 0) {
            lp[n] = n, primes.push_back(n), P++;
        }
        for (int i = 0; i < P && primes[i] <= lp[n] && n * primes[i] <= N; ++i) {
            lp[n * primes[i]] = primes[i], nxt[n * primes[i]] = n;
        }
    }

    return lp;
}

// Compute the number of unique prime divisors of all n<=N. O(N log log N)
auto num_prime_divisors_sieve(int N) {
    vector<int> num_prime_divisors;
    num_prime_divisors.assign(N + 1, 0);

    for (int n = 2; n <= N; n++) {
        if (num_prime_divisors[n] == 0) {
            for (int i = 1; i * n <= N; i++) {
                num_prime_divisors[i * n]++;
            }
        }
    }

    return num_prime_divisors;
}

// Compute the number of divisors of all n<=N, including trivial divisors. O(N log log N)
auto num_divisors_sieve(int N) {
    vector<int> num_divisors;
    num_divisors.assign(N + 1, 1);
    num_divisors[0] = 0;

    for (int n = 2; n <= N; n++) {
        if (num_divisors[n] == 1) {
            for (long m = n, k = 2; m <= N; m *= n, k++) {
                for (int i = 1; i * m <= N; i++) {
                    for (int b = 1; b < n && i * m <= N; i++, b++) {
                        num_divisors[i * m] *= k;
                    }
                }
            }
        }
    }

    return num_divisors;
}

// Compute the sum of divisors of all n<=N, including trivial divisors. O(N log log N)
auto sum_divisors_sieve(int N) {
    vector<long> sum_divisors;
    sum_divisors.assign(N + 1, 1);
    sum_divisors[0] = 0;

    for (int n = 2; n <= N; n++) {
        if (sum_divisors[n] == 1) {
            for (long m = n, k = 1 + n; m <= N; m *= n, k += m) {
                for (int i = 1; i * m <= N; i++) {
                    for (int b = 1; b < n && i * m <= N; i++, b++) {
                        sum_divisors[i * m] *= k;
                    }
                }
            }
        }
    }

    return sum_divisors;
}

// Compute phi(n) of all n<=N. O(N log log N)
auto phi_sieve(int N) {
    vector<int> phi;
    phi.resize(N + 1);
    iota(begin(phi), end(phi), 0);

    for (int n = 2; n <= N; n++) {
        if (phi[n] == n) {
            for (int i = n; i <= N; i += n) {
                phi[i] -= phi[i] / n;
            }
        }
    }

    return phi;
}

// Compute modinv(n, m) of all n<=N where m is a prime. O(N)
auto modinv_sieve(int N, int mod) {
    vector<int> inv;
    N = min(N, mod - 1);
    inv.resize(N + 1);
    inv[1] = 1;

    for (int n = 2; n <= N; n++) {
        inv[n] = mod - (mod / n) * inv[mod % n] % mod;
    }

    return inv;
}

// Compute log(n!) of all n<=N. O(N)
template <typename D = double>
auto logfac_sieve(int N) {
    vector<D> logfac;
    logfac.resize(N + 1);
    logfac[0] = logfac[1] = 1;

    for (int n = 2; n <= N; n++) {
        logfac[n] = logfac[n - 1] + log(D(n));
    }

    return logfac;
}

// Count primes in the range [L,R], both inclusive. Requires sieving first, such that
// primes[] contains all primes at least up to sqrt(R). O(R^1/2 + K log log K) where K=R-L
int count_primes(int64_t L, int64_t R, const vector<int>& primes) {
    assert(1 <= L && L <= R);
    vector<bool> isprime(R - L + 1, true);

    for (int64_t p : primes) {
        if (p * p > R)
            break;
        int64_t k = max((L + p - 1) / p, p);
        for (int64_t n = k * p; n <= R; n += p)
            isprime[n - L] = false;
    }
    isprime[0] = isprime[0] & (L > 1);

    return count(begin(isprime), end(isprime), true);
}

// Get primes in the range [L,R], both inclusive. Requires sieving first, such that
// primes[] contains all primes at least up to sqrt(R). O(R^1/2 + K log log K) where K=R-L
auto get_primes(int64_t L, int64_t R, const vector<int>& primes) {
    assert(1 <= L && L <= R);
    vector<bool> isprime(R - L + 1, true);

    for (int64_t p : primes) {
        if (p * p > R)
            break;
        int64_t k = max((L + p - 1) / p, p);
        for (int64_t n = k * p; n <= R; n += p)
            isprime[n - L] = false;
    }

    vector<long> new_primes;
    for (long n = L; n <= R; n++)
        if (isprime[n - L])
            new_primes.push_back(n);
    return new_primes;
}
