#pragma once

#include "possible_world.hpp"
#include <vector>
#include <random>
#include <functional>

/**
 * Edge of an uncertain graph.
 * 
 * @param id edge identifier. Since we consider undirected graphs, the edges (u, v) and (v, u) are assigned the same id.
 * @param dst destination node of reached by the edge.
 * @param prob probability of the edge.
 */
struct UncertainEdge {
    int id;
    int dst;
    double prob;
};

/**
 * Uncertain and undirected graph.
 * 
 * Since the graph might be unconnected, some nodes can have an empty vector of edges.
 *
 * @param n number of nodes.
 * @param m number of unique edges. The edges (u, v) and (v, u) are considered as the same edge.
 * @param adj adjacency vector with all edges. adj[u] contains all edges with u as source node.
 */
struct UncertainGraph {
    int n;
    int m;
    std::vector<std::vector<UncertainEdge>> adj;
};

/**
 * Samples a possible world from the input uncertain graph.
 *
 * @param uncertain_graph uncertain graph.
 * @param rng random number generator for reproducibility.
 *
 * @return possible world sampled from the input uncertain graph.
*/
PossibleWorld sample_world(const UncertainGraph &uncertain_graph, std::mt19937 &rng);

/**
 * Approximates the centrality of all the nodes in the input uncertain graph by running the input centrality function on an input number of Monte carlo samples.
 * 
 * @param uncertain_graph uncertain graph.
 * @param k number of possible worlds to sample.
 * @param in_world_centrality_fn address of the centrality function to apply in each possible world of the input uncertain graph.
 * @param rng random number generator for reproducibility.
 * 
 * @return vector of double where each component with index v stores the centrality of v in the input uncertain graph.
 */
std::vector<double> mc_centralities_uncertain_graph(const UncertainGraph &uncertain_graph, int k, const std::function<std::vector<double>(const PossibleWorld&)> &in_world_centrality_fn, std::mt19937 &rng);