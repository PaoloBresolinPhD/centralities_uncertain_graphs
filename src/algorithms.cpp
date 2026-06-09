#include "algorithms.hpp"

PossibleWorld sample_world(const UncertainGraph &uncertain_graph, std::mt19937 &rng) {

    // create a uniform distribution used to sample the edges
    std::uniform_real_distribution<double> distr(0.0, 1.0);

    // sample a probability value for each edge, which will be then compared to the edge probability
    std::vector<double> sampled_probs(uncertain_graph.m);
    for (int i = 0; i < uncertain_graph.m; ++i)
        sampled_probs[i] = distr(rng);

    // initialize the possible world that will contain only the sampled edges
    PossibleWorld world;
    world.n = uncertain_graph.n;
    world.adj = std::vector<std::vector<int>>(world.n);
    world.m = 0;

    // iterate through all the edges and sample them only if the probability is larger than the corresponding sampled value
    for (int i = 0; i < (int) uncertain_graph.n; ++i) {
        for (const UncertainEdge &e : uncertain_graph.adj[i]) {
            if (sampled_probs[e.id] < e.prob) {
                world.adj[i].push_back(e.dst);
                ++world.m;
            }
        }
    }

    // the number of undirected edges is half of the number of directed edges, since whenever (u, v) is sampled, also (v, u) is sampled
    world.m /= 2;

    return world;
}