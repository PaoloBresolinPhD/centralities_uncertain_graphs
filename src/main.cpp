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

    // CHECK THAT IT WORKED BY PRINTING THINGS
    // BUILD AND RUN
    
    // SAMPLE POSSIBLE WORLD

    return 0;
}