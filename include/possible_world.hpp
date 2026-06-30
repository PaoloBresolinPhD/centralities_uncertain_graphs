#pragma once

#include <vector>
#include <map>

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

/**
 * Performs a bfs to compute the distance from u to all the other nodes in the same connected component of u in the input possible world.
 * 
 * @param world possible world.
 * @param u source node.
 * 
 * @return map where the first element of each entry is the id of a node reached by the BFS from u and the second element is the distance from u to the node.
 */
std::map<int, int> bfs_distances(const PossibleWorld &world, int u);

/**
 * Finds all nodes reachable from u in the input possible world.
 * 
 * @param world possible world.
 * @param u source node.
 * 
 * @return vector of bool where each component with index v stores whether v is reachable from u in the input possible world or not.
 */
std::vector<bool> bfs_reachability(const PossibleWorld &world, int u);

/**
 * Finds the connected components in the input possible world.
 * 
 * @param world possible world.
 * 
 * @return vector of int where each component with index u stores the id of the connected component to which u belongs.
 */
std::vector<int> connected_components(const PossibleWorld &world);

/**
 * Computes the number of nodes in each connected component.
 * 
 * @param components vector with components ids for all nodes. components[v] contains the id of the connected component to which v belongs.
 * 
 * @return map with the size of each connected component. The entry with key i stores the number of nodes in the connected component with index i.
 */
std::map<int, int> components_sizes(const std::vector<int> &components);
