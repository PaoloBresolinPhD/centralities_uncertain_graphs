#include <vector>

struct UncertainGraph {
    
    // number of nodes
    int n;

    // number of undirected edges
    int m;

    // adjacency list: outgoing edges from node 0, outgoing edges from node 1, ...
    std::vector<int> adj;           // size: 2m
    
    // indices[i] is the index in adj of the distination node of the first edge outgoing from node i
    // adj[indices[i]] is the node reached from i with the first edge in the adjacency list of i
    std::vector<int> indices;       // size: n
    
    // edge_ids[i] is the id of the edge with destination adj[i]
    // it is necessary to assign the same edge id to two directed edges (u, v) and (v, u) in case of undirected graphs
    std::vector<int> edge_ids;      // size: 2m

    // edge_probs[i] is the existence probability of the edge with id edge_ids[i]
    std::vector<float> edge_probs;  // size: m
};