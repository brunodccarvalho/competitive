#include "test_utils.hpp"
#include "numeric/sieves.hpp"
#include "numeric/modnum.hpp"
#include "numeric/math.hpp"
#include "random.hpp"

void stress_test_primitive_root() {
    constexpr int N = 3000;
    auto small_primes = classic_sieve(N);

    for (int p : small_primes) {
        int g = primitive_root_prime(p);
        vector<int> seen(p + 1);
        seen[1] = true;
        long v = 1;
        for (int i = 1; i <= p - 2; i++) {
            v = v * g % p;
            assert(v != 1);
            seen[v] = true;
        }
        for (int i = 1; i <= p - 1; i++) {
            assert(seen[i]);
        }
        assert(!seen[p] && !seen[0]);
    }
}

void test_freq_primitive_root() {
    constexpr int N = 10'000'000;
    auto primes = classic_sieve(N);
    map<int, int> freq;
    for (int p : primes) {
        freq[primitive_root_prime(p)]++;
    }
    debug(freq);
}

void stress_test_jacobi() {
    for (long n = 1; n < 300; n += 2) {
        for (long m = 1; m < 300; m += 2) {
            if (gcd(n, m) == 1) {
                int reciprocity = ((n % 4) == 3 && (m % 4) == 3) ? -1 : 1;
                assert(jacobi(n, m) * jacobi(m, n) == reciprocity);
            }
        }
    }
}

void stress_test_miller_rabin() {
    constexpr long N = 4'000'000;
    auto primes = classic_sieve(N);

    vector<bool> small_prime(N + 1, false);
    for (int p : primes) {
        small_prime[p] = true;
    }
    for (long n = 1; n <= N; n++) {
        assert(small_prime[n] == miller_rabin(n));
    }

    for (int v : {5, 20, 300, 1000}) {
        long L = N * (N - v), R = N * (N - v + 5);
        auto large_primes = get_primes(L, R, primes);
        vector<bool> large_prime(R - L + 1, false);
        for (long n : large_primes) {
            large_prime[n - L] = true;
        }
        for (long n = L; n <= N; n++) {
            assert(large_prime[n - L] == miller_rabin(n));
        }
    }
}

int main() {
    RUN_BLOCK(test_freq_primitive_root());
    RUN_BLOCK(stress_test_primitive_root());
    RUN_BLOCK(stress_test_jacobi());
    RUN_BLOCK(stress_test_miller_rabin());
    return 0;
}
