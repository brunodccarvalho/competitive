#include "test_utils.hpp"
#include "numeric/modnum.hpp"
#include "numeric/sieves.hpp"

void speed_test_sieves() {
    map<pair<string, int>, string> table;

    for (int N : {31'600, 100'000, 316'000, 1'000'000, 3'160'000, 10'000'000, 31'600'000,
                  100'000'000}) {
        printcl(" speed sieves N={}", N);

        START(classic);
        classic_sieve(N);
        TIME(classic);
        table[{"classic", N}] = FORMAT_TIME(classic);

        START(least_prime);
        least_prime_sieve(N);
        TIME(least_prime);
        table[{"least_prime", N}] = FORMAT_TIME(least_prime);

        START(num_prime_divisors);
        num_prime_divisors_sieve(N);
        TIME(num_prime_divisors);
        table[{"num_prime_divisors", N}] = FORMAT_TIME(num_prime_divisors);

        START(num_divisors);
        num_divisors_sieve(N);
        TIME(num_divisors);
        table[{"num_divisors", N}] = FORMAT_TIME(num_divisors);

        START(sum_divisors);
        sum_divisors_sieve(N);
        TIME(sum_divisors);
        table[{"sum_divisors", N}] = FORMAT_TIME(sum_divisors);

        START(phi);
        phi_sieve(N);
        TIME(phi);
        table[{"phi", N}] = FORMAT_TIME(phi);

        START(modinv);
        modinv_sieve(N, 1'000'000'007);
        TIME(modinv);
        table[{"modinv", N}] = FORMAT_TIME(modinv);

        START(logfac);
        logfac_sieve(N);
        TIME(logfac);
        table[{"modinv", N}] = FORMAT_TIME(logfac);
    }

    print_time_table(table, "Sieves");
}

void unit_test_sieves() {
    constexpr int N = 100, M = 21;

    auto primes = classic_sieve(N);
    auto least = get<0>(least_prime_sieve(N));
    auto tau_primes = num_prime_divisors_sieve(N);
    auto tau = num_divisors_sieve(N);
    auto sigma = sum_divisors_sieve(N);
    auto phi = phi_sieve(N);
    auto modinv1e9 = modinv_sieve(N, 23);

    int ans[][M] = {
        {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73},
        {0, 0, 2, 3, 2, 5, 2, 7, 2, 3, 2, 11, 2, 13, 2, 3, 2, 17, 2, 19, 2},
        {0, 0, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 2, 1, 2, 2, 1, 1, 2, 1, 2},
        {0, 1, 2, 2, 3, 2, 4, 2, 4, 3, 4, 2, 6, 2, 4, 4, 5, 2, 6, 2, 6},
        {0, 1, 3, 4, 7, 6, 12, 8, 15, 13, 18, 12, 28, 14, 24, 24, 31, 18, 39, 20, 42},
        {0, 1, 1, 2, 2, 4, 2, 6, 4, 6, 4, 10, 4, 12, 6, 8, 8, 16, 6, 18, 8},
        {0, 1, 12, 8, 6, 14, 4, 10, 3, 18, 7, 21, 2, 16, 5, 20, 13, 19, 9, 17, 15},
    };

    for (int n = 0; n < M; n++) {
        assert(primes[n] == ans[0][n]);
        assert(least[n] == ans[1][n]);
        assert(tau_primes[n] == ans[2][n]);
        assert(tau[n] == ans[3][n]);
        assert(sigma[n] == ans[4][n]);
        assert(phi[n] == ans[5][n]);
        assert(modinv1e9[n] == ans[6][n]);
    }
}

void unit_test_num_divisors_sieve() {
    constexpr int N = 1'000'000;

    auto lp = get<0>(least_prime_sieve(N));
    auto divs = num_divisors_sieve(N);

    for (int n = 2; n <= N; n++) {
        int m = n, actual = 1;
        while (m > 1) {
            int i = 0, f = lp[m];
            do {
                m /= f, i++;
            } while (lp[m] == f);
            actual *= i + 1;
        }
        assert(actual == divs[n]);
    }
}

int main() {
    RUN_BLOCK(unit_test_sieves());
    RUN_BLOCK(unit_test_num_divisors_sieve());
    RUN_BLOCK(speed_test_sieves());
    return 0;
}
