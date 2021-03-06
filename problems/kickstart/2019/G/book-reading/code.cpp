#include <bits/stdc++.h>

using namespace std;

// *****

int N;            // number of pages
int T;            // number of pages torn out
int L;            // number of lazy readers
vector<int> torn; // pages torn out
vector<int> R;    // ith lazy reader reads multiples of R[i]

auto solve() {
    cin >> N >> T >> L;
    torn.resize(T);
    R.resize(L);

    for (int i = 0; i < T; ++i) {
        cin >> torn[i];
    }
    for (int i = 0; i < L; ++i) {
        cin >> R[i];
    }
    sort(begin(R), end(R));

    vector<bool> is_torn(N + 1, false);
    for (int t : torn) {
        is_torn[t] = true;
    }

    vector<int> torn_count(N + 1, 0);
    for (int i = 1; i <= N; ++i) {
        for (int j = i; j <= N; j += i) {
            torn_count[i] += !is_torn[j];
        }
    }

    uint64_t T = 0;
    for (int r : R) {
        T += torn_count[r];
    }
    return T;
}

// *****

int main() {
    unsigned T;
    cin >> T >> ws;
    for (unsigned t = 1; t <= T; ++t) {
        auto solution = solve();
        cout << "Case #" << t << ": " << solution << '\n';
    }
    return 0;
}
