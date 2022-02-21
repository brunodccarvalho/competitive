#include "../../code/hacking.hpp"

int T = 100;
vector<long> input;

int main(int argc, char** argv) {
    // if (argc >= 2) mt.seed(stoull(argv[1])), argc--, argv++; // seed mt
    // if (argc >= 2) T = stoi(argv[1]), argc--, argv++; // read a batch size
    // while (argc >= 2) input.push_back(stoll(argv[1])), argc--, argv++; // read numbers

    // ofstream ans("answer.txt");
    putln(T);
    for (int t = 1; t <= T; t++) {
        // putln("::hack", t);
        int N = rand_int(1, 5);
        putln(N);
        for (int i = 0; i < N; i++) {
            int a = rand_int(1, 100'000);
            int b = rand_int(1, 100'000);
            int c = rand_int(1, 100'000);
            putln(a, b, c);
        }
    }
    return 0;
}
