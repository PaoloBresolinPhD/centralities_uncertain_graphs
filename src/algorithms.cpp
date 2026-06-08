#include "algorithms.hpp"

PossibleWorld sample_world(const UncertainGraph &uncertain_graph, std::mt19937 &rng) {

    // create a uniform distribution used to sample the edges
    std::uniform_real_distribution<double> distr(0.0, 1.0);

    // is_found[i] stores whether the edge with id i has already been considered for sampling
    std::vector<bool> is_found(uncertain_graph.m, false);

    // is_sampled[i] stores whether the edge with id i has been sampled or not
    std::vector<bool> is_sampled(uncertain_graph.m, false);

    // initialize the possible world that will contain only sampled edges
    PossibleWorld world;
    world.n = uncertain_graph.n;
    world.adj = std::vector<std::vector<int>>(world.n);
    world.m = 0;

    // iterate through the nodes
    for (int i = 0; i < (int) uncertain_graph.n; ++i) {

        // iterate through all edges with the current node as source node
        for (const UncertainEdge &e : uncertain_graph.adj[i]) {

            // check whether the current edge has already been considered
            if (is_found[e.id]) {

                // add it to the possible world if it has been sampled, discard it otherwise
                if (is_sampled[e.id]) {
                    world.adj[i].push_back(e.dst);
                }
            }

            // sample the edge otherwise
            else {
                is_found[e.id] = true;
                if (distr(rng) < e.prob) {
                    world.adj[i].push_back(e.dst);
                    ++world.m;
                    is_sampled[e.id] = true;
                }
            }
        }
    }

    return world;
}