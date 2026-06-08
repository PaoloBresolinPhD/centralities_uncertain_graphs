#pragma once

#include <vector>

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
 * Represents a possible world of an uncertain graph.
 * 
 * Since the graph might be unconnected, some nodes can have an empty vector of edges.
 *
 * @param n number of nodes in the possible world.
 * @param m number of undirected edges in the possible world.
 * @param adj adjacency list of the possible world. adj[u] has all nodes reached by edges with u as source node.
*/
struct PossibleWorld {
    int n;
    int m;
    std::vector<std::vector<int>> adj;
};