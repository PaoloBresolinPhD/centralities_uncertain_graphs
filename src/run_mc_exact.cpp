#include "uncertain_graph.hpp"
#include "lin_algorithms.hpp"
#include "utils.hpp"
#include <iostream>
#include <omp.h>
#include <chrono>

int main(int argc, char* argv[]) {

    // default command line parameters
    std::string input_path = "";
    std::string output_path = "";
    int k = 10000;
    int random_seed = 27;
    int n_threads = 8;

    // parse the command line arguments
    for (int i = 1; i < argc - 1; i += 2) {
        std::string flag = argv[i];
        std::string value = argv[i + 1];
        if (flag == "--input_path")
            input_path = value;
        else if (flag == "--output_path")
            output_path = value;
        else if (flag == "-k")
            k = std::stoi(value);
        else if (flag == "-r")
            random_seed = std::stoi(value);
        else if (flag == "--n_threads")
            n_threads = std::stoi(value);
        else {
            std::cerr << "Error: unknown argument " << flag << ".\n";
            return 1;
        }
    }

    // validate that all required arguments are provided as input
    if (input_path.empty()) {
        std::cerr << "Error: missing required argument --input_path.";
        return 1;
    }
    else if (output_path.empty()) {
        std::cerr << "Error: missing required argument --output_path.";
        return 1;
    }

    // parameters
    double p = 0.5;
    std::mt19937 rng(random_seed);

    // set the number of omp threads to the input value
    omp_set_num_threads(n_threads);

    // load the graph
    std::cout << "Loading the graph from the file " << input_path << "...\n";
    UncertainGraph uncertain_graph = load_uncertain_graph_no_probs(input_path, p);

    // print some information about the loaded graph
    std::cout << "Number of nodes: " << uncertain_graph.n << "\n";
    std::cout << "Number of edges: " << uncertain_graph.m << "\n";

    // assign uniform probabilities to the edges in the graph
    assign_uniform_edge_probs(uncertain_graph, rng);

    // run the exact algorithm on the loaded uncertain graph
    auto start_exact = std::chrono::high_resolution_clock::now();
    std::vector<double> exact_centralities = mc_centralities_uncertain_graph(uncertain_graph, k, &exact_lin_world, rng);
    auto end_exact = std::chrono::high_resolution_clock::now();

    // compute and print the time required to compute the centralities
    std::chrono::duration<double> elapsed_exact = end_exact - start_exact;
    std::cout << "Running time of MC + exact (s): " << elapsed_exact.count();

    // save the computed centralities in the output file in tsv format
    std::vector<std::vector<double>> table;
    table.push_back(exact_centralities);
    std::vector<std::string> cols;
    cols.push_back("Exact");
    save_centralities_tsv(output_path, table, cols);

    return 0;
}