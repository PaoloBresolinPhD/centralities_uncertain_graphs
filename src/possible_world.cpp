#include "possible_world.hpp"
#include <queue>
#include <cmath>

std::vector<int> bfs_distances(const PossibleWorld &world, int u) {
    
    // initialize the vector that will contain all distances with the sentinel value -1
    std::vector<int> distances(world.n, -1);

    // initialize the queue of the bfs with the source node u
    std::queue<int> queue;
    queue.push(u);

    // set to 0 the distance from u to itself
    distances[u] = 0;

    // perform a bfs starting from u
    while (!queue.empty()) {

        // dequeue the next node
        int v = queue.front();
        queue.pop();

        // iterate through its neighbors
        for (int w : world.adj[v]) {

            // visit the node only if not visited yet
            if (distances[w] == -1) {
                
                // store the distance from u to w and enqueue w
                distances[w] = distances[v] + 1;
                queue.push(w);
            }
        }

    }

    return distances;
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
