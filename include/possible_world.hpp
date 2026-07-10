#pragma once

#include <random>
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

/**
 * Computes an upper bound to the maximum diameter in the input possible world.
 * The maximum diameter corresponds to the maximum possible distance between two nodes in any connected component of the input possible world.
 * In each connected component, a node u is sampled uniformly at random and its distance d(u, v) from the farthest node v is computed.
 * Since the diameter of the connected component is <= 2 * d(u, v), then 2 * d(u, v) is an upper bound to the diameter for the considered connected component.
 * h nodes are sampled and the minimum value 2 * d(u, v) is considered for the considered connected component.
 * The maximum upper bound among all connected components is finally returned as upper bound for the maximum diameter across the connected components of the input graph.
 * 
 * @param possible_world possible world.
 * @param h number of nodes to sample from each connected component.
 * @param rng random number generator for reproducibility.
 * 
 * @return integer representing the maximum diameter in the input possible world.
 */
int upper_bound_max_diameter(const PossibleWorld &world, int h, std::mt19937 &rng);
