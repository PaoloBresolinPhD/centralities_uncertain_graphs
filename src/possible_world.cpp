#include "possible_world.hpp"
#include <queue>
#include <cmath>
#include <climits>

void bfs_distances(const PossibleWorld &world, int u, std::vector<int> &distances, std::vector<int> &reached_nodes) {
    
    // clear the vector with reached nodes
    reached_nodes.clear();

    // add the the source node to the vector
    reached_nodes.push_back(u);

    // index of the next node to be expanded in the bfs
    int head = 0;

    // set the distance from u to itself to 0
    distances[u] = 0;

    // perform a bfs starting from u
    while (head < (int) reached_nodes.size()) {

        // get the next node to expand
        int v = reached_nodes[head++];

        // iterate through the neighbors of the node to expand
        for (int w : world.adj[v]) {

            // visit the node only if not visited yet
            if (distances[w] == -1) {
                
                // store the distance from u to w
                distances[w] = distances[v] + 1;
                
                // insert w among the reached nodes
                reached_nodes.push_back(w);
            }
        }
    }
}

std::vector<bool> bfs_reachability(const PossibleWorld &world, int u) {
    
    // vector that will store whether each node is reachable from u in the input possible world
    std::vector<bool> reachable(world.n, false);

    // initialize the queue of the bfs with the source node u
    std::queue<int> queue;
    queue.push(u);

    // set u as reachable from itself
    reachable[u] = true;

    // perform a bfs starting from u
    while (!queue.empty()) {

        // dequeue the next node
        int v = queue.front();
        queue.pop();

        // iterate through its neighbors
        for (int w : world.adj[v]) {

            // visit the node only if it has not been reached yet
            if (!reachable[w]) {
                
                // set w as reachable and enqueue it
                reachable[w] = true;
                queue.push(w);
            }
        }

    }

    return reachable;
}

std::vector<int> connected_components(const PossibleWorld &world) {

    // initialize the vector that will store the connected components ids with the sentinel value -1
    std::vector<int> components(world.n, -1);

    // initialize the id for the current connected component
    int id = 0;

    // iterate through all nodes
    for (int u = 0; u < world.n; ++u) {

        // start a bfs from u only if u has not been reached by any bfs yet
        if (components[u] == -1) {
            
            // find all nodes reachable from u
            std::vector<bool> reachable = bfs_reachability(world, u);
            
            // assign the same component id to each node reached
            for (int v = 0; v < world.n; ++v)
                if (reachable[v])
                    components[v] = id;
            
            // increment the component id
            ++id;
        }
    }

    return components;
}

std::map<int, int> components_sizes(const std::vector<int> &components) {

    // map that will store the size of each connected component
    std::map<int, int> sizes;

    // iterate through all nodes
    for (int i = 0; i < components.size(); ++i) {

        // increment the count if the connected component of the current node is already in the map; set it to 1 otherwise
        if (sizes.find(components[i]) != sizes.end())
            ++sizes[components[i]];
        else
            sizes[components[i]] = 1;
    }

    return sizes;
}

int upper_bound_max_diameter(const PossibleWorld &world, int h, std::mt19937 &rng) {

    // compute the connected components
    std::vector<int> comps = connected_components(world);

    // create a map with the ids of the connected components as keys and vectors with the corresponding nodes as values
    std::map<int, std::vector<int>> comps_map;
    for (int u = 0; u < (int) comps.size(); ++u)
        comps_map[comps[u]].push_back(u);

    // initialize the variable that will contain the maximum across all connected components of the minimum of the maximum distances from the sampled nodes
    int diameter_ub = 0;

    // initialize the vector that will contain the distances from each sampled node to all other nodes in the input possible world
    std::vector<int> distances(world.n, -1);

    // initialize the vector that will contain the nodes reached fromm a bfs call
    std::vector<int> reached_nodes;
    reached_nodes.reserve(world.n);

    // iterate through the connected components
    for (auto &curr_comp : comps_map) {

        // define a uniform distrubution of integers, used to sample nodes from the current connected component
        std::uniform_int_distribution<int> distr(0, curr_comp.second.size() - 1);

        // initialize the variable that will contain the minimum of the maximum distances from the sampled nodes
        int min_ub_comp = INT_MAX;

        // iterate h times
        for (int i = 0; i < h; ++i) {

            // compute the distances from a node sampled uniformly at random and all the other nodes in the connected component
            bfs_distances(world, curr_comp.second[distr(rng)], distances, reached_nodes);

            // find the maximum distance
            int curr_max_dist = 0;
            for (int reached : reached_nodes) {
                if (distances[reached] > curr_max_dist)
                    curr_max_dist = distances[reached];
                if (distances[reached] > min_ub_comp)
                    break;
            }

            // update the minimum across all sampled nodes
            if (curr_max_dist < min_ub_comp)
                min_ub_comp = curr_max_dist;   
            
            // reset the distances of the nodes reached by the bfs to the sentinel value -1
            for (int reached : reached_nodes)
                distances[reached] = -1;
        }

        // if the minimum found upper bound is larger than the the upper bound found in the connected components found so far, then update it
        if (min_ub_comp > diameter_ub)
            diameter_ub = min_ub_comp;
    }

    // return the upper bound, which is twice the maximum distance found by sampling
    return 2 * diameter_ub;
}
