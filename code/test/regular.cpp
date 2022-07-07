#include "test_utils.hpp"
#include "lib/graph_generator.hpp"

void scaling_test_random_regular() {
    vector<vector<stringable>> table;
    table.push_back({"n", "k", "E", "time/edge", "time"});

    auto run = [&](int n, int k) {
        printcl("scaling test random regular n,k={},{}", n, k);
        START(regular);
        LOOP_FOR_DURATION_TRACKED_RUNS (1s, now, runs) {
            auto g = random_regular(n, k);
        }
        TIME(regular);

        long E = 1L * runs * (n * k / 2);
        table.push_back({n, k, E, FORMAT_EACH(regular, E), FORMAT_EACH(regular, runs)});
    };

    run(300, 100);
    run(300, 200);
    run(25, 6);
    run(25, 10);
    run(36, 6);
    run(36, 8);
    run(36, 10);
    run(80, 5);
    run(80, 9);
    run(80, 13);
    run(400, 12);
    run(400, 16);
    run(400, 20);
    run(10000, 30);
    run(10000, 40);
    run(10000, 50);
    run(50000, 60);
    run(50000, 120);

    print_time_table(table, "Scaling random regular");
}

int main() {
    RUN_BLOCK(scaling_test_random_regular());
    return 0;
}
