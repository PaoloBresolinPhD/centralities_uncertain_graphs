#include "algorithms.hpp"
#include "utils.hpp"
#include <iostream>

int main(int argc, char* argv[]) {

    // return an error if the number of arguments is not correct
    if (argc != 3) {
        std::cerr << "Error: invalid number of arguments.\n";
        std::cerr << "Usage: " << argv[0] << " <file_path> <random_seed>\n";
        return 1;
    }

    // parameters
    double p = 0.5;
    std::mt19937 rng(std::stoi(argv[2]));

    // extract the path to the input file with the dataset
    std::string input_file_path = argv[1];

    // load the graph
    std::cout << "Loading the graph from the file " << input_file_path << "...\n";
    UncertainGraph uncertain_graph = load_uncertain_graph_no_probs(input_file_path, p);

    // // print some information about the loaded graph
    // std::cout << "Number of nodes: " << uncertain_graph.n << "\n";
    // std::cout << "Number of edges: " << uncertain_graph.m << "\n";
    // std::cout << "\n\nNumber of edges per node: \n";
    // for (int u = 0; u < (int) uncertain_graph.adj.size(); ++u) {
    //     std::cout << u << ": " << uncertain_graph.adj[u].size() << "\n";
    // }
    // int n_example_nodes = 10;
    // std::cout << "\n\n\nExample of the adjacency list of the first " << n_example_nodes << " nodes:\n";
    // for (int u = 0; u < (int) uncertain_graph.adj.size(); ++u) {
    //     std::cout << "\n\n" << u << ":\n";
    //     for (const UncertainEdge &e : uncertain_graph.adj[u]) {
    //         std::cout << "    dst: " << e.dst << "\n";
    //         std::cout << "    id: " << e.id << "\n";
    //         std::cout << "    prob: " << e.prob << "\n";
    //     }
    // }

    // assign random probabilities to the edges in the graph
    assign_uniform_edge_probs(uncertain_graph, rng);

    // // print some information about the probabilities
    // std::cout << "\n\nProbabilities assigned to some edges:\n\n";
    // for (int i = 0; i < 10; ++i)
    //     for (const auto &e : uncertain_graph.adj[i])
    //         std::cout << "Edge: " << e.id << " | p: " << e.prob << "\n";
    // double avg_prob = 0;
    // for (int i = 0; i < uncertain_graph.n; ++i)
    //     for (const auto &e : uncertain_graph.adj[i])
    //         avg_prob += e.prob;
    // std::cout << "\n\nAverage assigned edge probability: " << avg_prob / (2 * uncertain_graph.m) << "\n";
    
    // // sample a couple of possible worlds
    // PossibleWorld w_1 = sample_world(uncertain_graph, rng);
    // PossibleWorld w_2 = sample_world(uncertain_graph, rng);

    // // print some information about the sampled possible worlds
    // std::cout << "Uncertain graph n: " << uncertain_graph.n << "\n";
    // std::cout << "Uncertain graph m: " << uncertain_graph.m << "\n";
    // std::cout << "\nPossible world 1 n: " << w_1.n << "\n";
    // std::cout << "Possible world 1 m: " << w_1.m << "\n";
    // std::cout << "\nPossible world 2 n: " << w_2.n << "\n";
    // std::cout << "Possible world 2 m: " << w_2.m << "\n";
    // std::vector<int> a = {0, 1, 27, 814};
    // for (const int u : a) {
    //     std::cout << "\n\nNodes reached by " << u << " in the uncertain graph:\n";
    //     for (const auto &v : uncertain_graph.adj[u])
    //         std::cout << v.dst << "\n";
    //     std::cout << "\n\nNodes reached by " << u << " in the possible world 1:\n";
    //     for (const int v : w_1.adj[u])
    //         std::cout << v << "\n";
    //     std::cout << "\n\nNodes reached by " << u << " in the possible world 2:\n";
    //     for (const int v : w_2.adj[u])
    //         std::cout << v << "\n";
    // }


    return 0;
}