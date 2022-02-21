#include "test_utils.hpp"
#include "lib/event_time_tracker.hpp"
#include "struct/online_dynamic_connectivity.hpp"
#include "struct/pbds.hpp"
#include "lib/slow_graph.hpp"

void random_test_dynacon() {
    enum Action { LINK, CUT, LINK_CUT };
    vector<string> event_names = {"LINK", "CUT", "LINK_CUT"};

    event_time_tracker tracker({1000, 1990, 1000});

    constexpr int N = 3000;
    online_dynamic_connectivity dynacon(N);
    slow_graph slow(N);
    int S = N, E = 0;
    ordered_set<pair<int, int>> edges;

    auto has_edge = [&](int u, int v) { return edges.find(minmax(u, v)) != edges.end(); };
    auto add_edge = [&](int u, int v) {
        if (cointoss(0.5))
            swap(u, v);

        edges.insert(minmax(u, v)), E++;
        slow.link(u, v);

        tracker.start_event(LINK);
        S -= dynacon.link(u, v);
        tracker.time();
    };
    auto rem_edge = [&](int u, int v) {
        if (cointoss(0.5))
            swap(u, v);

        edges.erase(minmax(u, v)), E--;
        slow.cut(u, v);

        tracker.start_event(CUT);
        S += dynacon.cut(u, v);
        tracker.time();
    };

    // Start off with random edge additions
    for (int i = 0; i < N / 3; i++) {
        print_regular(i, N, 100, "initial edges... S,E={}", S, E);
        auto [u, v] = different<int>(1, N);
        if (!has_edge(u, v)) {
            add_edge(u, v);
        }
    }

    assert(S == slow.num_components());
    deque<string> labels;

    LOOP_FOR_DURATION_TRACKED_RUNS (30s, now, runs) {
        print_time(now, 30s, "test dynacon ({} runs, S,E={},{})", runs, S, E);

        int event = tracker.next_event();
        string label;

        switch (event) {
        case LINK: {
            auto [u, v] = different<int>(1, N);
            if (!has_edge(u, v)) {
                add_edge(u, v);
                label = format("[{}] LINK {}--{}", slow.num_components(), u, v);
            }
        } break;
        case CUT: {
            if (E > 0) {
                auto [u, v] = *edges.find_by_order(intd(0, E - 1)(mt));
                rem_edge(u, v);
                label = format("[{}] CUT {}--{}", slow.num_components(), u, v);
            }
        } break;
        case LINK_CUT: {
            auto [u, v] = different<int>(1, N);
            if (cointoss(0.5))
                swap(u, v);
            if (has_edge(u, v)) {
                rem_edge(u, v);
                label = format("[{}] CUT {}--{}", slow.num_components(), u, v);
            } else {
                add_edge(u, v);
                label = format("[{}] LINK {}--{}", slow.num_components(), u, v);
            }
        } break;
        }

        if (!label.empty()) {
            labels.push_back(label);
        }
        if (labels.size() > 20u) {
            labels.pop_front();
        }
        if (S != slow.num_components()) {
            printcl("{}", fmt::join(labels, "{}\n"));
        }
        assert(S == slow.num_components());
    }

    tracker.pretty_log(event_names);
}

int main() {
    RUN_BLOCK(random_test_dynacon());
    return 0;
}
