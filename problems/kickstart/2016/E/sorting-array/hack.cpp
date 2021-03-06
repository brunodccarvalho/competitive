#include <bits/stdc++.h>

using namespace std;

int factorial(int N) {
    int n = N;
    while (N > 1)
        n *= --N;
    return n;
}

int main(int argc, char **argv) {
    int N = argc >= 2 ? atoi(argv[1]) : 6;

    vector<int> vec(N);
    iota(vec.begin(), vec.end(), 1);

    printf("%d\n", factorial(N));

    do {
        printf("%d 3\n%d", N, vec[0]);
        for (int i = 1; i < N; ++i)
            printf(" %d", vec[i]);
        printf("\n");
    } while (next_permutation(vec.begin(), vec.end()));

    return 0;
}
