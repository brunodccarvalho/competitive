#include "test_utils.hpp"
#include "graphs/topology.hpp"
#include "lib/graph_operations.hpp"
#include "lib/graph_formats.hpp"

void unit_test_tree_centers() {
    using vi = vector<int>;
    int V;
    vector<array<int, 2>> g;
    vector<vector<int>> tree;
    vector<int> centers, diameter;

    V = 14;
    g = {{0, 2},   {1, 2},  {2, 5}, {3, 4}, {4, 5}, {11, 10}, {12, 10},
         {13, 10}, {10, 8}, {8, 9}, {6, 9}, {9, 7}, {5, 9}};
    tree = make_adjacency_lists_undirected(V, g);
    centers = find_tree_centers(tree);
    diameter = find_tree_diameter(tree);
    print("centers: {}\n", centers);
    print("diameter: {}\n", diameter);
    assert(centers.size() == 1 && centers[0] == 9);

    V = 11;
    g = {{0, 2}, {1, 2}, {2, 3}, {3, 4}, {3, 5}, {5, 6}, {5, 7}, {5, 8}, {8, 10}, {8, 9}};
    tree = make_adjacency_lists_undirected(V, g);
    centers = find_tree_centers(tree);
    diameter = find_tree_diameter(tree);
    sort(begin(centers), end(centers));
    print("centers: {}\n", centers);
    print("diameter: {}\n", diameter);
    assert(centers.size() == 2 && centers == vi({3, 5}));
}

int main() {
    RUN_SHORT(unit_test_tree_centers());
    return 0;
}
