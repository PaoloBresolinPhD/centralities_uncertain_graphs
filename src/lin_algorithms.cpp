#include "lin_algorithms.hpp"
#include <cmath>

std::vector<double> exact_lin_world(const PossibleWorld &world) {

    // initialize the vector that will store the Lin's indices
    std::vector<double> centralities(world.n, 0);

    // find the connected component to which each node belongs
    std::vector<int> components = connected_components(world);

    // compute the number of nodes in each connected component
    std::map<int, int> comp_sizes = components_sizes(components);

    // iterate through all the nodes
    for (int u = 0; u < world.n; ++u) {

        // perform a bfs from u
        std::vector<int> distances = bfs_distances(world, u);

        // update the centralities of all nodes reached by the bfs
        for (int v = 0; v < world.n; ++v)
            if (distances[v] != -1)
                centralities[v] += distances[v];
    }

    // compute the actual Lin's indices
    for (int u = 0; u < world.n; ++u)
        if (centralities[u] != 0)
            centralities[u] = std::pow(comp_sizes[components[u]] - 1, 2) / ((world.n - 1) * centralities[u]);

    return centralities;
}