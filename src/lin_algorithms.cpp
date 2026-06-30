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
        std::map<int, int> distances = bfs_distances(world, u);

        // update the centralities of all nodes reached by the bfs
        for (auto reached : distances)
            centralities[reached.first] += reached.second;
    }

    // compute the actual Lin's indices
    for (int u = 0; u < world.n; ++u)
        if (centralities[u] != 0)
            centralities[u] = std::pow(comp_sizes[components[u]] - 1, 2) / ((world.n - 1) * centralities[u]);

    return centralities;
}

std::vector<int> uniform_sample_with_replacement(const std::vector<int> &vec, int l, std::mt19937 &rng) {

    // initialize the vector that will contain the sampled values
    std::vector<int> sample(l);

    // initialize a uniform distribution of integers in the desired range
    std::uniform_int_distribution<int> distr(0, (int) vec.size() - 1);

    // sample l value uniformly at random with replacement
    for (int i = 0; i < l; ++i)
        sample[i] = vec[distr(rng)];
    
    return sample;
}

std::vector<double> ew_lin_world(const PossibleWorld &world, int l, double c, std::mt19937 &rng)  {

    // initialize the vector that will store the Lin's indices
    std::vector<double> centralities(world.n, 0);

    // find the connected component to which each node belongs and the sizes of the connected components
    std::vector<int> node_components = connected_components(world);
    std::map<int, int> comp_sizes = components_sizes(node_components);

    // iterate through each connected component
    for (auto &comp : comp_sizes) {

        // if the connected component is smaller than c, then 0 is returned as centrality for all the nodes in it
        if (comp.second >= c) {

            // find the ids of the nodes in the current connected component
            std::vector<int> comp_nodes;
            comp_nodes.reserve(comp.second);
            for (int u = 0; u < (int) node_components.size(); ++u)
                if (node_components[u] == comp.first)
                    comp_nodes.push_back(u);
            
            // initialize the vector with the nodes from which start a BFS as the vector with all nodes in the connected component
            std::vector<int> bfs_nodes = comp_nodes;

            // if the component is larger than l, we sample l nodes uniformly at random with replacement from the connected component
            if (comp.second > l)
                bfs_nodes = uniform_sample_with_replacement(comp_nodes, l, rng);
            
            // update the centralities of all the ndoes in the connected component by runnign a BFS from each node in the bfs_nodes vector
            for (int u : bfs_nodes) {
                std::map<int, int> distances = bfs_distances(world, u);
                for (auto &reached : distances)
                    centralities[reached.first] += reached.second;
            }

            // normalize the centralities
            if (comp.second > l)
                for (int u : comp_nodes)
                    if (centralities[u] > 0)
                        centralities[u] = l * std::pow(comp.second - 1, 2) / ( (world.n - 1) * centralities[u] * comp.second );
            else
                for (int u : comp_nodes)
                    if (centralities[u] > 0)
                        centralities[u] = std::pow(comp.second - 1, 2) / ( (world.n - 1) * centralities[u] );
        }
    }

    return centralities;
}