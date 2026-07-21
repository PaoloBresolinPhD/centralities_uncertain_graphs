#include "lin_algorithms.hpp"
#include "utils.hpp"
#include <cmath>

std::vector<double> exact_harmonic_world(const PossibleWorld &world) {

    // initialize the vector that will store the harmonic centralities
    std::vector<double> centralities(world.n, 0);

    // iterate through all the nodes
    for (int u = 0; u < world.n; ++u) {

        // perform a bfs from u
        std::map<int, int> distances = bfs_distances(world, u);

        // update the centralities of all nodes reached by the bfs
        for (auto reached : distances)
            if (reached.second > 0)
                centralities[reached.first] += 1.0 / reached.second;
    }

    // compute the actual harmonic centralities
    for (int u = 0; u < world.n; ++u)
        if (centralities[u] != 0)
            centralities[u] /= (world.n - 1);

    return centralities;
}

std::vector<double> ew_harmonic_world(const PossibleWorld &world, int l, std::mt19937 &rng)  {

    // initialize the vector that will store the harmonic centralities
    std::vector<double> centralities(world.n, 0);

    // initialize a vector with all the node ids
    std::vector<int> nodes(world.n);
    for (int u = 0; u < world.n; ++u)
        nodes[u] = u;
    
    // initialize the vector with the nodes from which to start a BFS with all nodes
    std::vector<int> bfs_nodes(nodes);

    // if l is smaller than the number of nodes in the input possible world, then sample l nodes
    if (world.n > l)
        bfs_nodes = uniform_sample_with_replacement(nodes, l, rng);

    // update the centralities of all the ndoes by runnign a BFS from each node in the bfs_nodes vector
    for (int u : bfs_nodes) {
        std::map<int, int> distances = bfs_distances(world, u);
        for (auto &reached : distances)
            if (reached.second > 0)
                centralities[reached.first] += 1.0 / reached.second;
    }

    // normalize the centralities
    if (world.n > l)
        for (int u = 0; u < world.n; ++u)
            if (centralities[u] > 0)
                centralities[u] *= ((double) world.n / l / (world.n - 1));
    else
        for (int u = 0; u < world.n; ++u)
            if (centralities[u] > 0)
                centralities[u] /= (world.n - 1);
    
    // clamp the centralities to the maximum possible value            
    for (int u = 0; u < world.n; ++u)
        if (centralities[u] > 1.0)
            centralities[u] = 1.0;

    return centralities;
}

std::map<int, double> harmonic_pps_sample(const PossibleWorld &world, const std::vector<int> &nodes, int l, double p_s, std::mt19937 &rng) {
    
    // extract a Poisson sample from the nodes in the input possible world
    std::vector<int> initial_poisson_sample = poisson_sample(nodes, std::vector<double>(world.n, p_s), rng);

    // initialize the map with the pps values for all the nodes
    std::map<int, double> gamma;
    for (int u : nodes)
        gamma[u] = 1.0 / world.n;

    // iterate through the nodes of the Poisson sample
    for (int u : initial_poisson_sample) {

        // compute the distances from u to all eachable nodes
        std::map<int, int> distances = bfs_distances(world, u);

        // compute the sum of the inverse of the distance from u to each reachable node
        double sum_distances = 0.0;
        for (auto &entry : distances)
            if (entry.second > 0)
                sum_distances += 1.0 / entry.second;
        
        // update the pps values of all the reached nodes   
        for (auto &entry : distances)
            if (entry.second > 0)
                gamma[entry.first] = std::max(gamma[entry.first], 1.0 / (sum_distances * entry.second));
    }

    // normalize the pps probabilities
    for (int u : nodes)
        gamma[u] = std::min(1.0, l * gamma[u]);
    
    // create the sample by Poisson sampling of the nodes using the computed probabilities
    std::vector<double> poisson_probs;
    poisson_probs.reserve(world.n);
    for (int u : nodes)
        poisson_probs.push_back(gamma[u]);
    std::vector<int> sampled_nodes = poisson_sample(nodes, poisson_probs, rng);

    // create a map with the sampled nodes and their corresponding probabilities
    std::map<int, double> sample;
    for (int u : sampled_nodes)
        sample[u] = gamma[u];
    
    return sample;
}

std::vector<double> pps_harmonic_world(const PossibleWorld &world, int k, int l, double delta, std::mt19937 &rng) {

    // create a vector with all the nodes in the input possble world
    std::vector<int> nodes(world.n);
    for (int u = 0; u < world.n; ++u)
        nodes[u] = u;
    
    // initialize the vector that will store the harmonic centralities
    std::vector<double> centralities(world.n, 0);

    // if the number of nodes in the input possible world is larger than l, then we sample O(l) nodes using PPS sampling
    if (world.n > l) {

        // compute the Poisson probability p_s
        double p_s = 2.0 / world.n * std::log(4 * k * world.n / delta);
        
        // extract the PPS sample
        std::map<int, double> map_sample = harmonic_pps_sample(world, nodes, l, p_s, rng);
    
        // update the centralities of all the nodes by running a BFS from each sampled node
        for (auto &entry : map_sample) {
            std::map<int, int> distances = bfs_distances(world, entry.first);
            for (auto &reached : distances)
                if (reached.second > 0)
                    centralities[reached.first] += 1.0 / (reached.second * entry.second);
        }
    }

    // else, run a bfs from each node in the input possible world and compute the exact centrality values for the nodes
    else {
        for (int u : nodes) {
            std::map<int, int> distances = bfs_distances(world, u);
            for (auto &reached : distances)
                if (reached.second > 0)
                    centralities[reached.first] += 1.0 / reached.second;
        }
    }

    // normalize the centralities
    for (int u : nodes) {
        if (centralities[u] > 0) {
            centralities[u] /= (world.n - 1);
            centralities[u] = std::min(1.0, centralities[u]);
        }
    }

    return centralities;
}
