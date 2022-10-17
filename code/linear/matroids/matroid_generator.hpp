#pragma once

#include "matroids.hpp"
#include "random.hpp"
#include "lib/graph_generator.hpp"
#include "numeric/modnum.hpp"

enum class MatroidType {
    UNIFORM,
    COLORFUL,
    PARTITION,
    GRAPHIC,
    INCREMENTAL_GRAPHIC,
    INCREMENTAL_BICIRCULAR,
    INCREMENTAL_BINARY,
    INCREMENTAL_VECTOR,
    END,
};

string to_string(MatroidType distr) {
    static const string names[] = {
        "uniform"s,
        "colorful"s,
        "partition"s,
        "graphic"s,
        "incremental-graphic"s,
        "incremental-bicircular"s,
        "incremental-binary"s,
        "incremental-vector"s,
    };
    return names[int(distr)];
}

auto rand_matroid_type() {
    static discrete_distribution<int> dist({2, 4, 7, 4, 3, 2, 4, 3});
    return MatroidType(dist(mt));
}

auto rand_exchange_matroid_type() {
    static discrete_distribution<int> dist({2, 4, 7, 4, 0, 0, 0, 0});
    return MatroidType(dist(mt));
}

auto rand_uniform_matroid(int n) {
    int k = rand_grav<int>(1, n, 2);
    return uniform_matroid(n, k);
}

auto rand_colorful_matroid(int n, int C = 0) {
    C = C > 0 ? C : rand_wide<int>(1, n, -3);
    auto color = rands_unif<colorful_matroid::Color>(n, 0, C - 1);
    return colorful_matroid(C, color);
}

auto rand_partition_matroid(int n, int C = 0) {
    C = C > 0 ? C : rand_wide<int>(1, n, -3);
    auto color = rands_unif<partition_matroid::Color>(n, 0, C - 1);
    vector<partition_matroid::Limit> count(C);
    for (int c : color) {
        count[c]++;
    }
    auto limit = rands_unif<partition_matroid::Limit>(C, 1, (n + C - 1) / C);
    for (int c = 0; c < C; c++) {
        limit[c] = min(limit[c], count[c]);
    }
    return partition_matroid(color, limit);
}

auto rand_incremental_graphic_matroid(int n, int V = 0) {
    int required = ceil((1 + sqrt(1 + 8 * n)) / 2.0);
    V = V > 0 ? V : rand_wide<int>(required, n + 1, -2);
    auto g = random_exact_undirected(V, n);
    shuffle(begin(g), end(g), mt);
    return incremental_graphic_matroid(V, g);
}

auto rand_graphic_matroid(int n, int V = 0) {
    int required = ceil((1 + sqrt(1 + 8 * n)) / 2.0);
    V = V > 0 ? V : rand_wide<int>(required, n + 1, -2);
    auto g = random_exact_undirected(V, n);
    shuffle(begin(g), end(g), mt);
    return graphic_matroid(V, g);
}

auto rand_incremental_bicircular_matroid(int n, int V = 0) {
    int required = ceil((1 + sqrt(1 + 8 * n)) / 2.0);
    V = V > 0 ? V : rand_wide<int>(required, n + 1, -2);
    auto g = random_exact_undirected(V, n);
    shuffle(begin(g), end(g), mt);
    return incremental_bicircular_matroid(V, g);
}

template <typename T = int64_t>
auto rand_incremental_binary_matroid(int n, int B = 0) {
    static constexpr int MAX = 8 * sizeof(T) - 1;
    static boold coind(0.5);

    // Build a basis of size B
    B = B > 0 ? min(B, n) : rand_wide<int>(1, min(n, MAX), +1);

    // Build a basis using these maximum set bits
    vector<int> msb = int_sample<int>(B, 0, MAX);
    vector<T> basis(B);
    for (int i = 0; i < B; i++) {
        T bit = T(1) << msb[i];
        basis[i] = rand_unif<T>(bit, 2 * (bit - 1) + 1);
    }

    // Add the basis to the vector space, and combine vectors from the basis for the rest
    vector<T> space(n);
    copy(begin(basis), end(basis), begin(space));
    for (int i = B; i < n; i++) {
        for (int j = 0; j < B; j++) {
            space[i] ^= coind(mt) ? 0 : basis[j];
        }
    }

    shuffle(begin(space), end(space), mt);
    return incremental_binary_matroid<T>(space);
}

template <typename T = modnum<1'000'000'007>>
auto rand_incremental_vector_matroid(int n, int S = 0, int B = 0) {
    using U = typename T::u32;
    static constexpr U MAX = T::MOD - 1;
    static boold coind(0.5);

    // Use vectors of size S
    S = S > 0 ? S : rand_wide<int>(1, min(n, 12), +1);

    // Build a basis of size B
    B = B > 0 ? min({B, S, n}) : rand_wide<int>(1, min(n, S), -1);

    // Build a basis using these maximum set "bits"
    vector<int> msb = int_sample<int>(B, 0, S);
    vector<vector<T>> basis(B);
    for (int i = 0; i < B; i++) {
        basis[i].resize(S);
        basis[i][msb[i]] = rand_unif<U>(1, MAX);
        for (int k = msb[i] + 1; k < S; k++) {
            basis[i][k] = rand_wide<U>(0, MAX, -1);
        }
    }

    // Add the basis to the vector space, and combine vectors from the basis
    vector<vector<T>> space(n);
    copy(begin(basis), end(basis), begin(space));
    for (int i = B; i < n; i++) {
        space[i].resize(S);
        for (int j = 0; j < B; j++) {
            if (coind(mt)) {
                auto m = rand_unif<U>(1, MAX);
                for (int k = 0; k < S; k++) {
                    space[i][k] += m * basis[j][k];
                }
            }
        }
    }

    shuffle(begin(space), end(space), mt);
    return incremental_vector_matroid<T>(space);
}

using any_matroid_t = matroid_variant<uniform_matroid,                                //
                                      colorful_matroid,                               //
                                      partition_matroid,                              //
                                      incremental_graphic_matroid,                    //
                                      graphic_matroid,                                //
                                      incremental_bicircular_matroid,                 //
                                      incremental_binary_matroid<int>,                //
                                      incremental_binary_matroid<int64_t>,            //
                                      incremental_binary_matroid<uint>,               //
                                      incremental_binary_matroid<uint64_t>,           //
                                      incremental_vector_matroid<modnum<998244353>>,  //
                                      incremental_vector_matroid<modnum<1000000007>>, //
                                      incremental_vector_matroid<modnum<100000009>>,  //
                                      incremental_vector_matroid<modnum<10007>>>;

using any_matroid_exchange_t = matroid_variant<uniform_matroid,   //
                                               colorful_matroid,  //
                                               partition_matroid, //
                                               graphic_matroid>;

any_matroid_t rand_matroid(MatroidType type, int n) {
    switch (type) {
    case MatroidType::UNIFORM:
        return rand_uniform_matroid(n);
    case MatroidType::COLORFUL:
        return rand_colorful_matroid(n);
    case MatroidType::PARTITION:
        return rand_partition_matroid(n);
    case MatroidType::GRAPHIC:
        return rand_graphic_matroid(n);
    case MatroidType::INCREMENTAL_GRAPHIC:
        return rand_incremental_graphic_matroid(n);
    case MatroidType::INCREMENTAL_BICIRCULAR:
        return rand_incremental_bicircular_matroid(n);
    case MatroidType::INCREMENTAL_BINARY:
        return rand_incremental_binary_matroid(n);
    case MatroidType::INCREMENTAL_VECTOR:
        return rand_incremental_vector_matroid(n);
    default:
        throw runtime_error("Invalid matroid type");
    }
}

any_matroid_exchange_t rand_exchange_matroid(MatroidType type, int n) {
    switch (type) {
    case MatroidType::UNIFORM:
        return rand_uniform_matroid(n);
    case MatroidType::COLORFUL:
        return rand_colorful_matroid(n);
    case MatroidType::PARTITION:
        return rand_partition_matroid(n);
    case MatroidType::GRAPHIC:
        return rand_graphic_matroid(n);
    default:
        throw runtime_error("Invalid exchange matroid type");
    }
}

auto rand_matroid(int n) {
    return rand_matroid(rand_matroid_type(), n); //
}
auto rand_exchange_matroid(int n) {
    return rand_exchange_matroid(rand_exchange_matroid_type(), n);
}
