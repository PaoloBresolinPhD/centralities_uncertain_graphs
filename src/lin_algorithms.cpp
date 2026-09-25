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

    // initialize the vector that will contain the distances from each node to all other nodes in the input possible world
    std::vector<int> distances(world.n, -1);

    // initialize the vector that will contain the nodes reached fromm a bfs call
    std::vector<int> reached_nodes;
    reached_nodes.reserve(world.n);

    // iterate through all the nodes
    for (int u = 0; u < world.n; ++u) {

        // perform a bfs from u
        bfs_distances(world, u, distances, reached_nodes);

        // update the centralities of all nodes reached by the bfs
        for (int reached : reached_nodes)
            centralities[reached] += distances[reached];

        // reset the distances of the reached nodes to the sentinel value -1
        for (int reached : reached_nodes)
            distances[reached] = -1;
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

    // initialize the vector that will contain the distances from each sampled node to all other nodes in the input possible world
    std::vector<int> distances(world.n, -1);

    // initialize the vector that will contain the nodes reached from a bfs call
    std::vector<int> reached_nodes;
    reached_nodes.reserve(world.n);

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
                bfs_distances(world, u, distances, reached_nodes);
                for (int reached : reached_nodes)
                    centralities[reached] += distances[reached];
                for (int reached : reached_nodes)
                    distances[reached] = -1;
            }

            // normalize the centralities
            if (comp.second > l) {
                for (int u : comp_nodes)
                    if (centralities[u] > 0)
                        centralities[u] = l * std::pow(comp.second - 1, 2) / ( (world.n - 1) * centralities[u] * comp.second );
            }
            else {
                for (int u : comp_nodes)
                    if (centralities[u] > 0)
                        centralities[u] = std::pow(comp.second - 1, 2) / ( (world.n - 1) * centralities[u] );
            }
            
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

    // initialize the vector that will contain the distances from each sampled node to all other nodes in the input possible world
    std::vector<int> distances(world.n, -1);

    // initialize the vector that will contain the nodes reached fromm a bfs call
    std::vector<int> reached_nodes;
    reached_nodes.reserve(world.n);

    // iterate through the nodes of the Poisson sample
    for (int u : initial_poisson_sample) {

        // compute the distances from u to all the nodes in the considered connected component
        bfs_distances(world, u, distances, reached_nodes);

        // compute the sum of distances from u to all the nodes in the same connected component
        double sum_distances = 0.0;
        for (int reached : reached_nodes)
            sum_distances += distances[reached];
        
        // update the pps values of all the nodes in the connected component    
        for (int v : conn_comp_nodes)
            gamma[v] = std::max(gamma[v], distances[v] / sum_distances);

        // reset the distances of the reached nodes to the sentinel value -1
        for (int reached : reached_nodes)
            distances[reached] = -1;
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

    // initialize the vector that will contain the distances from each sampled node to all other nodes in the input possible world
    std::vector<int> distances(world.n, -1);

    // initialize the vector that will contain the nodes reached from a bfs call
    std::vector<int> reached_nodes;
    reached_nodes.reserve(world.n);

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
                bfs_distances(world, entry.first, distances, reached_nodes);
                for (int reached : reached_nodes)
                    centralities[reached] += distances[reached] / entry.second;
                for (int reached : reached_nodes)
                    distances[reached] = -1;
            }
        }

        // else, run a bfs from each node in the current connected component and compute the exact centrality values for the nodes
        else {
            for (int u : comp_nodes) {
                bfs_distances(world, u, distances, reached_nodes);
                for (int reached : reached_nodes)
                    centralities[reached] += distances[reached];
                for (int reached : reached_nodes)
                    distances[reached] = -1;
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

std::vector<double> exact_lin_world_query(const PossibleWorld &world, const std::vector<int> &query) {

    // initialize the vector that will store the Lin's indices of the query nodes
    std::vector<double> centralities(query.size(), 0);

    // initialize the vector that will contain the distances from each node in the query to all other nodes in the input possible world
    std::vector<int> distances(world.n, -1);

    // initialize the vector that will contain the nodes reached fromm a bfs call
    std::vector<int> reached_nodes;
    reached_nodes.reserve(world.n);

    // iterate through all the nodes in the query
    for (int i = 0; i < (int) query.size(); ++i) {

        // perform a bfs from the current query node
        bfs_distances(world, query[i], distances, reached_nodes);

        // compute the centrality for the current node
        for (int reached : reached_nodes)
            centralities[i] += distances[reached];

        // reset the distances of the reached nodes to the sentinel value -1
        for (int reached : reached_nodes)
            distances[reached] = -1;

        // compute the actual Lin's indicex by normalizing
        if (centralities[i] != 0)
            centralities[i] = std::pow((double) reached_nodes.size() - 1, 2) / ((world.n - 1) * centralities[i]);
    }

    return centralities;
}

std::vector<double> ew_lin_world_query(const PossibleWorld &world, const std::vector<int> &query, int l, double c, std::mt19937 &rng)  {

    // initialize the vector that will store the Lin's indices
    std::vector<double> centralities(query.size(), 0);

    // find the connected component to which each node belongs and the sizes of the connected components
    std::vector<int> node_components = connected_components(world);
    std::map<int, int> comp_sizes = components_sizes(node_components);

    // initialize the vector that will contain the distances from each sampled node to all other nodes in the input possible world
    std::vector<int> distances(world.n, -1);

    // initialize the vector that will contain the nodes reached from a bfs call
    std::vector<int> reached_nodes;
    reached_nodes.reserve(world.n);

    // group the query nodes by the component they belong to
    std::map<int, std::vector<int>> query_by_comp;
    for (int i = 0; i < (int) query.size(); ++i)
        query_by_comp[node_components[query[i]]].push_back(i);

    // iterate through each connected component with at least one query node
    for (auto &comp : query_by_comp) {

        // if the component is larger than c, then we start some BFSs; otherwise, 0 is returned for each query node in the connected component
        if (comp_sizes[comp.first] >= c) {

            // find the ids of the nodes in the current connected component
            std::vector<int> comp_nodes;
            comp_nodes.reserve(comp_sizes[comp.first]);
            for (int u = 0; u < (int) node_components.size(); ++u)
                if (node_components[u] == comp.first)
                    comp_nodes.push_back(u);

            // if there are more than l query nodes in the component, then we sample l nodes uniformly at random with replacement from the connected component
            if ((int) comp.second.size() > l) {

                // vector that will contain the nodes from which a BFS has to be run
                std::vector<int> bfs_nodes;
                bfs_nodes.reserve(l);

                // sample l nodes from those in the current connected component
                bfs_nodes = uniform_sample_with_replacement(comp_nodes, l, rng);

                // update the centralities of all the query nodes in the connected component by runnign a BFS from each sampled node
                for (int u : bfs_nodes) {
                    bfs_distances(world, u, distances, reached_nodes);
                    for (int i = 0; i < (int) comp.second.size(); ++i)
                        centralities[comp.second[i]] += distances[query[comp.second[i]]];
                    for (int reached : reached_nodes)
                        distances[reached] = -1;
                }

                // normalize the centralities
                for (int i = 0; i < (int) comp.second.size(); ++i)
                    if (centralities[comp.second[i]] > 0)
                        centralities[comp.second[i]] = l * std::pow(comp_sizes[comp.first] - 1, 2) / ( (world.n - 1) * centralities[comp.second[i]] * comp_sizes[comp.first] );
                
                // clamp the centralities to the maximum possible value
                for (int i = 0; i < (int) comp.second.size(); ++i)
                    if (centralities[comp.second[i]] > 0)
                        centralities[comp.second[i]] = std::min( (double) (comp_sizes[comp.first] - 1) / (world.n - 1), centralities[comp.second[i]]);
            }

            // else, we compute the exact measure for each query node, since it requires less than l BFS runs
            else {

                // iterate through the query nodes in the current connected component
                for (int i = 0; i < (int) comp.second.size(); ++i) {

                    // compute the sum of distances for each query node
                    bfs_distances(world, query[comp.second[i]], distances, reached_nodes);
                    for (int reached : comp_nodes)
                        centralities[comp.second[i]] += distances[reached];
                    for (int reached : comp_nodes)
                        distances[reached] = -1;
                    
                    // compute the centrality by normalizing the sum
                    if (centralities[comp.second[i]] > 0)
                        centralities[comp.second[i]] = std::pow(comp_sizes[comp.first] - 1, 2) / ( (world.n - 1) * centralities[comp.second[i]] );
                }
            }
        }
    }

    return centralities;
}

std::vector<double> pps_lin_world_query(const PossibleWorld &world, const std::vector<int> &query, int k, int l, double delta, std::mt19937 &rng) {

    // initialize the vector that will store the Lin's indices for the query nodes
    std::vector<double> centralities(query.size(), 0);

    // find the connected component to which each node belongs and the sizes of the connected components
    std::vector<int> node_components = connected_components(world);
    std::map<int, int> comp_sizes = components_sizes(node_components);

    // initialize the vector that will contain the distances from each sampled node to all other nodes in the input possible world
    std::vector<int> distances(world.n, -1);

    // initialize the vector that will contain the nodes reached from a bfs call
    std::vector<int> reached_nodes;
    reached_nodes.reserve(world.n);

    // group the query nodes by the component they belong to
    std::map<int, std::vector<int>> query_by_comp;
    for (int i = 0; i < (int) query.size(); ++i)
        query_by_comp[node_components[query[i]]].push_back(i);

    // iterate through each connected component with at least one query node
    for (auto &comp : query_by_comp) {

        // if there are more than l query nodes in the component, then we sample l nodes using pps sampling on the connected component
        if ((int) comp.second.size() > l) {

            // find the ids of the nodes in the current connected component
            std::vector<int> comp_nodes;
            comp_nodes.reserve(comp_sizes[comp.first]);
            for (int u = 0; u < (int) node_components.size(); ++u)
                if (node_components[u] == comp.first)
                    comp_nodes.push_back(u);

            // compute the Poisson probability p_s
            double p_s = 2.0 / comp_sizes[comp.first] * std::log(4 * k * comp_sizes.size() / delta);
            
            // extract the PPS sample
            std::map<int, double> map_sample = lin_pps_sample(world, comp_nodes, l, p_s, rng);
        
            // update the centralities of all the query nodes in the current connected component by running a BFS from each sampled node
            for (auto &entry : map_sample) {
                bfs_distances(world, entry.first, distances, reached_nodes);
                for (int i = 0; i < (int) comp.second.size(); ++i)
                    centralities[comp.second[i]] += distances[query[comp.second[i]]] / entry.second;
                for (int reached : reached_nodes)
                    distances[reached] = -1;
            }
        }

        // else, compute the exact centrality values for the query nodes in the current connected component, as it requires less than l BFS calls
        else {
            for (int i = 0; i < (int) comp.second.size(); ++i) {
                bfs_distances(world, query[comp.second[i]], distances, reached_nodes);
                for (int reached : reached_nodes)
                    centralities[comp.second[i]] += distances[reached];
                for (int reached : reached_nodes)
                    distances[reached] = -1;
            }
        }

        // normalize the centralities
        for (int i = 0; i < (int) comp.second.size(); ++i) {
            if (centralities[comp.second[i]] > 0) {
                centralities[comp.second[i]] = std::pow(comp_sizes[comp.first] - 1, 2) / ( (world.n - 1) * centralities[comp.second[i]] );
                centralities[comp.second[i]] = std::min( (double) (comp_sizes[comp.first] - 1) / (world.n - 1), centralities[comp.second[i]]); // maximum possible value
            }
        }
    }

    return centralities;
}
