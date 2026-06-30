#include "uncertain_graph.hpp"
#include <queue>
#include <cmath>
#include <omp.h>

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
    for (int i = 0; i < uncertain_graph.n; ++i) {
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

std::vector<double> mc_centralities_uncertain_graph(const UncertainGraph &uncertain_graph, int k, const std::function<std::vector<double>(const PossibleWorld&)> &in_world_centrality_fn, std::mt19937 &rng) {

    // initialize the centralities computed in the possible worlds
    std::vector<double> centralities_uncertain(uncertain_graph.n, 0.0);

    // generate a different random seed for each thread
    int n_threads = omp_get_max_threads();
    std::vector<uint32_t> seeds(n_threads);
    for (auto &s : seeds)
        s = rng();

    // parallel region
    #pragma omp parallel
    {

        // initialize the random number generator for a single thread
        std::mt19937 thread_rng(seeds[omp_get_thread_num()]);

        // vector that stores the centralieties computed by a signle thread
        std::vector<double> thread_centralities(uncertain_graph.n, 0.0);
        
        // iterate through k sampled possible worlds
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < k; ++i) {

            // sample a possible world
            PossibleWorld world = sample_world(uncertain_graph, thread_rng);

            // compute the centralites in the sampled possible world
            std::vector<double> centralities_world = in_world_centrality_fn(world);

            // update the vector with the centralities for the current thread
            for (int u = 0; u < uncertain_graph.n; ++u)
                thread_centralities[u] += centralities_world[u];
            
        }

        // update the vector shared among all threads, waiting until no thread is writing
        #pragma omp critical
        for (int u = 0; u < uncertain_graph.n; ++u)
                centralities_uncertain[u] += thread_centralities[u];
    }

    // take the mean of the computed centralities
    for (int u = 0; u < uncertain_graph.n; ++u)
        centralities_uncertain[u] /= k;

    return centralities_uncertain;
}