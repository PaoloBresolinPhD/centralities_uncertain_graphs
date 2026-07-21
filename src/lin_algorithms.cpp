#include "lin_algorithms.hpp"
#include "utils.hpp"
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
            
            // clamp the centralities to the maximum possible value            
            for (int u : comp_nodes)
                if (centralities[u] > 0)
                    centralities[u] = std::min( (double) (comp.second - 1) / (world.n - 1), centralities[u]);
        }
    }

    return centralities;
}

std::map<int, double> lin_pps_sample(const PossibleWorld &world, const std::vector<int> &conn_comp_nodes, int l, double p_s, std::mt19937 &rng) {

    // return an empty vector if the size of the connected component is <= 1
    if (conn_comp_nodes.size() <= 1)
        return std::map<int, double>();
    
    // extract a Poisson sample from the nodes in the input connected component
    std::vector<int> initial_poisson_sample = poisson_sample(conn_comp_nodes, std::vector<double>(conn_comp_nodes.size(), p_s), rng);

    // initialize the map with the pps values for all the nodes in the input connected component
    std::map<int, double> gamma;
    for (int u : conn_comp_nodes)
        gamma[u] = 1.0 / conn_comp_nodes.size();

    // iterate through the nodes of the Poisson sample
    for (int u : initial_poisson_sample) {

        // compute the distances from u to all the nodes in the considered connected component
        std::map<int, int> distances = bfs_distances(world, u);

        // compute the sum of distances from u to all the nodes in the same connected component
        double sum_distances = 0.0;
        for (auto &entry : distances)
            sum_distances += entry.second;
        
        // update the pps values of all the nodes in the connected component    
        for (int v : conn_comp_nodes)
            gamma[v] = std::max(gamma[v], distances[v] / sum_distances);
    }

    // normalize the pps probabilities
    for (int u : conn_comp_nodes)
        gamma[u] = std::min(1.0, l * gamma[u]);
    
    // create the sample by Poisson sampling of the nodes using the computed probabilities
    std::vector<double> poisson_probs;
    poisson_probs.reserve(conn_comp_nodes.size());
    for (int u : conn_comp_nodes)
        poisson_probs.push_back(gamma[u]);
    std::vector<int> sampled_nodes = poisson_sample(conn_comp_nodes, poisson_probs, rng);

    // create a map with the sampled nodes and their corresponding probabilities
    std::map<int, double> sample;
    for (int u : sampled_nodes)
        sample[u] = gamma[u];
    
    return sample;
}

std::vector<double> pps_lin_world(const PossibleWorld &world, int k, int l, double delta, std::mt19937 &rng) {

    // initialize the vector that will store the Lin's indices
    std::vector<double> centralities(world.n, 0);

    // find the connected component to which each node belongs and the sizes of the connected components
    std::vector<int> node_components = connected_components(world);
    std::map<int, int> comp_sizes = components_sizes(node_components);

    // iterate through each connected component
    for (auto &comp : comp_sizes) {

        // find the ids of the nodes in the current connected component
        std::vector<int> comp_nodes;
        comp_nodes.reserve(comp.second);
        for (int u = 0; u < (int) node_components.size(); ++u)
            if (node_components[u] == comp.first)
                comp_nodes.push_back(u);

        // if the component is larger than l, then we sample O(l) nodes from the connected component using PPS sampling
        if (comp.second > l) {

            // compute the Poisson probability p_s
            double p_s = 2.0 / comp.second * std::log(4 * k * comp_sizes.size() / delta);
            
            // extract the PPS sample
            std::map<int, double> map_sample = lin_pps_sample(world, comp_nodes, l, p_s, rng);
        
            // update the centralities of all the nodes in the connected component by running a BFS from each sampled node
            for (auto &entry : map_sample) {
                std::map<int, int> distances = bfs_distances(world, entry.first);
                for (auto &reached : distances)
                    centralities[reached.first] += reached.second / entry.second;
            }
        }

        // else, run a bfs from each node in the current connected component and compute the exact centrality values for the nodes
        else {
            for (int u : comp_nodes) {
                std::map<int, int> distances = bfs_distances(world, u);
                for (auto &reached : distances)
                    centralities[reached.first] += reached.second;
            }
        }

        // normalize the centralities
        for (int u : comp_nodes) {
            if (centralities[u] > 0) {
                centralities[u] = std::pow(comp.second - 1, 2) / ( (world.n - 1) * centralities[u] );
                centralities[u] = std::min( (double) (comp.second - 1) / (world.n - 1), centralities[u]); // maximum possible value
            }
        }
    }

    return centralities;
}
