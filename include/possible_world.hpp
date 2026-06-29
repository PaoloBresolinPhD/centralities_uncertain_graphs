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
 * Performs a bfs to compute the distance from u to all the other nodes in the input possible world.
 * Since a possible world might be disconnected, then the distance between u and a node not reachable from u in the input possible world are set to the sentinel value -1.
 * 
 * @param world possible world.
 * @param u source node.
 * 
 * @return vector of int where the component with index v stores the distance between u and v in the input possible world.
 */
std::vector<int> bfs_distances(const PossibleWorld &world, int u);

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
