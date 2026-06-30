#include "uncertain_graph.hpp"
#include "lin_algorithms.hpp"
#include "utils.hpp"
#include <iostream>
#include <omp.h>
#include <chrono>
#include <filesystem>
#include <utility>

std::pair<std::vector<double>, double> run_method(UncertainGraph &uncertain_graph, const std::function<std::vector<double>(const PossibleWorld&)> &in_world_centrality_fn, int k, const std::vector<double> &baseline_centralities, std::mt19937 &rng) {

    // run the input method
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<double> estimated_centralities = mc_centralities_uncertain_graph(uncertain_graph, k, in_world_centrality_fn, rng);
    auto end_time = std::chrono::high_resolution_clock::now();

    // compute the centralities errors and add them to the table
    std::vector<double> centralities_errors(uncertain_graph.n);
    for (int u = 0; u < uncertain_graph.n; ++u)
        centralities_errors[u] = std::abs(baseline_centralities[u] - estimated_centralities[u]);

    // compute and append the time required to compute the centralities in seconds
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    // return a pair with the errors and the elapsed time
    return std::pair<std::vector<double>, double>(centralities_errors, elapsed_time.count());
}

void run_single_experiment(UncertainGraph &uncertain_graph, std::string &output_dir_path, int k_baseline, double c_ew, int k, int l, std::mt19937 &rng) {

    // ---------- INITIALIZATION ----------

    // number of methods to compare
    int n_methods = 3;

    // vector that will contain the name of each method
    std::vector<std::string> method_names;
    method_names.reserve(n_methods);

    // table that will contain a column for each algorithm with a centrality value for each node
    std::vector<std::vector<double>> table;
    table.reserve(n_methods - 1);    // we do not save the errors of the exact baseline with respect to itself, since they are 0

    // vector that will contain the execution times of the methods
    std::vector<double> exec_times;
    exec_times.reserve(n_methods);
    
    // ---------- BASELINE EXACT ----------

    // append the name of the method
    method_names.push_back("baseline_exact");

    // run the exact algorithm on the input uncertain graph with a baseline (i.e., large) value of k
    std::cout << "Running exact with k = " << k_baseline << std::endl;
    auto start_time_baseline = std::chrono::high_resolution_clock::now();
    std::vector<double> exact_centralities_baseline = mc_centralities_uncertain_graph(uncertain_graph, k_baseline, exact_lin_world, rng);
    auto end_time_baseline = std::chrono::high_resolution_clock::now();

    // compute and append the time required to compute the centralities
    std::chrono::duration<double> elapsed_time_baseline = end_time_baseline - start_time_baseline;
    exec_times.push_back(elapsed_time_baseline.count());

    // ---------- EXACT ----------

    // append the name of the method
    method_names.push_back("exact");

    // run the exact algorithm on the input uncertain graph with k
    std::cout << "Running exact with k = " << k << std::endl;
    auto [exact_errors, exact_elapsed_time] = run_method(uncertain_graph, exact_lin_world, k, exact_centralities_baseline, rng);

    // append the computed errors and the execution time of the current method
    table.push_back(exact_errors);
    exec_times.push_back(exact_elapsed_time);
    
    // ---------- EW ----------

    // append the name of the method
    method_names.push_back("ew");

    // set c to be at least 2
    c_ew = std::max<double> (c_ew, 2);

    // create the lambda adapter for the ew_lin_world function, since it requires additional input arguments
    auto ew_lin_world_adapter = [&](const PossibleWorld &world) {
        return ew_lin_world(world, l, c_ew, rng);
    };

    // run the ew algorithm on the input uncertain graph with k
    std::cout << "Running ew with k = " << k << ", l = " << l << " and c = " << c_ew << std::endl;
    auto [ew_errors, ew_elapsed_time] = run_method(uncertain_graph, ew_lin_world_adapter, k, exact_centralities_baseline, rng);

    // append the computed errors and the execution time of the current method
    table.push_back(ew_errors);
    exec_times.push_back(ew_elapsed_time);

    // ---------- SAVE THE RESULTS ----------

    // path to the folder where to save the results
    std::filesystem::path output_dir(output_dir_path);

    // save the computed centralities in the output file in tsv format
    std::filesystem::path centralities_path = output_dir / "errors.tsv";
    std::vector<std::string> table_cols(n_methods - 1);
    for (int i = 1; i < n_methods; ++i)
        table_cols[i - 1] = method_names[i];
    save_centralities_tsv(centralities_path, table, table_cols);

    // save the execution times of the methods in tsv format
    std::filesystem::path times_path = output_dir / "execution_times.tsv";
    save_vector_tsv(times_path, exec_times, method_names);
}

int main(int argc, char* argv[]) {

    // default command line parameters
    std::string input_path = "";
    std::string output_path = "";
    double c_ew = 1;
    int k_baseline = 10000;
    int k = 1000;
    int l = 10;
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
        else if (flag == "--c_ew")
            c_ew = std::stod(value);
        else if (flag == "--k_baseline")
            k_baseline = std::stoi(value);
        else if (flag == "--k")
            k = std::stoi(value);
        else if (flag == "--l")
            l = std::stoi(value);
        else if (flag == "--r")
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
    std::cout << "Loading the graph from the file " << input_path << "..." << std::endl;
    UncertainGraph uncertain_graph = load_uncertain_graph_no_probs(input_path, p);

    // print some information about the loaded graph
    std::cout << "Number of nodes: " << uncertain_graph.n << std::endl;
    std::cout << "Number of edges: " << uncertain_graph.m << std::endl;

    // assign uniform probabilities to the edges in the graph
    assign_uniform_edge_probs(uncertain_graph, rng);

    // run the experiments
    run_single_experiment(uncertain_graph, output_path, k_baseline, c_ew, k, l, rng);


    // TODO:
    //     - implement the pps algorithm for the lin's index;
    //     - add the implemented algorithm to this file and test it to see whether the output errors make sense;
    //     - change this code to envelop the single experiment into a for that repeats the experiment n times.
    //       Each of the n single experiments returns the table with the errors and the vector with the running times.
    //       Once a single experiment is executed the errors and times are summed up across experiment repetitions.
    //       After that all single experiments have finished, the errors and times are averaged and saved to file;
    //     - compute the expected error epsilon, once set c, l, k, delta from the theory and compare it with the experimental results;
    //     - implement everything for the harmonic centrality.

    return 0;
}