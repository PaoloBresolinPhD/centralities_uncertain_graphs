#include "uncertain_graph.hpp"
#include "lin_algorithms.hpp"
#include "utils.hpp"
#include <iostream>
#include <omp.h>
#include <chrono>
#include <filesystem>
#include <utility>
#include <algorithm>
#include <numeric>

std::tuple<std::vector<double>, double, double, double, double> run_method(UncertainGraph &uncertain_graph, const std::function<std::vector<double>(const PossibleWorld&)> &in_world_centrality_fn, int k, const std::vector<double> &baseline_centralities, std::mt19937 &rng) {

    // run the input method
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<double> estimated_centralities = mc_centralities_uncertain_graph(uncertain_graph, k, in_world_centrality_fn, rng);
    auto end_time = std::chrono::high_resolution_clock::now();

    // compute and append the time required to compute the centralities in seconds
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    // compute the centralities errors and add them to the table
    std::vector<double> centralities_errors(uncertain_graph.n);
    for (int u = 0; u < uncertain_graph.n; ++u)
        centralities_errors[u] = std::abs(baseline_centralities[u] - estimated_centralities[u]);

    // compute the minimum, maximum and average error
    auto min_max_pair = std::minmax_element(centralities_errors.begin(), centralities_errors.end());
    double min_error = *(min_max_pair.first);
    double max_error = *(min_max_pair.second);
    double sum_errors = std::accumulate(centralities_errors.begin(), centralities_errors.end(), 0.0);
    double avg_error = sum_errors / centralities_errors.size();

    // return a tuple with the errors, the elapsed time, the minimum error, the maximum error and the averge error
    return std::tuple<std::vector<double>, double, double, double, double>(centralities_errors, elapsed_time.count(), min_error, max_error, avg_error);
}

void run_single_experiment(UncertainGraph &uncertain_graph, std::filesystem::path &output_dir_path, int k_baseline, int k_mc, int k_ew, int l_ew, int c_ew, int k_pps, int l_pps, int p_s_factor, std::mt19937 &rng, int exp_rep) {

    // ---------- INITIALIZATION ----------

    // map that will contain the method names as keys and the vector with ceentrality errors of all nodes as value
    std::map<std::string, std::vector<double>> table;

    // map that will contain the execution times of the methods
    std::map<std::string, double> exec_times;

    // map that will contain the data to store in the summary tsv
    std::map<std::string, double> summary_map;
    summary_map["exp_rep"] = exp_rep;
    
    // ---------- BASELINE EXACT ----------

    // run the exact algorithm on the input uncertain graph with a baseline (i.e., large) value of k
    std::cout << "Running exact with k = " << k_baseline << "..." << std::endl;
    auto start_time_exact = std::chrono::high_resolution_clock::now();
    std::vector<double> exact_centralities_baseline = mc_centralities_uncertain_graph(uncertain_graph, k_baseline, exact_lin_world, rng);
    auto end_time_exact = std::chrono::high_resolution_clock::now();

    // compute and append the time required to compute the centralities
    std::chrono::duration<double> exact_elapsed_time = end_time_exact - start_time_exact;
    exec_times["baseline_exact"] = exact_elapsed_time.count();

    // update the sumary map
    summary_map["k_baseline"] = k_baseline;
    summary_map["exact_time"] = exact_elapsed_time.count();

    // ---------- MC ----------

    // run the exact algorithm on the input uncertain graph with k
    std::cout << "Running mc with k = " << k_mc << "..." << std::endl;
    auto [mc_errors, mc_elapsed_time, mc_min, mc_max, mc_avg] = run_method(uncertain_graph, exact_lin_world, k_mc, exact_centralities_baseline, rng);

    // add the computed errors and the execution time of the current method
    table["mc"] = mc_errors;
    exec_times["mc"] = mc_elapsed_time;

    // update the summary map
    summary_map["k_mc"] = k_mc;
    summary_map["mc_time"] = mc_elapsed_time;
    summary_map["mc_error_min"] = mc_min;
    summary_map["mc_error_max"] = mc_max;
    summary_map["mc_error_avg"] = mc_avg;
    
    // ---------- EW ----------

    // create the lambda adapter for the ew_lin_world function, since it requires additional input arguments
    auto ew_lin_world_adapter = [&](const PossibleWorld &world) {
        return ew_lin_world(world, l_ew, c_ew, rng);
    };

    // run the ew algorithm on the input uncertain graph with k
    std::cout << "Running ew with k = " << k_ew << ", l = " << l_ew << " and c = " << c_ew << "..." << std::endl;
    auto [ew_errors, ew_elapsed_time, ew_min, ew_max, ew_avg] = run_method(uncertain_graph, ew_lin_world_adapter, k_ew, exact_centralities_baseline, rng);

    // add the computed errors and the execution time of the current method
    table["ew"] = ew_errors;
    exec_times["ew"] = ew_elapsed_time;

    // update the summary map
    summary_map["k_ew"] = k_ew;
    summary_map["l_ew"] = l_ew;
    summary_map["c_ew"] = c_ew;
    summary_map["ew_time"] = ew_elapsed_time;
    summary_map["ew_error_min"] = ew_min;
    summary_map["ew_error_max"] = ew_max;
    summary_map["ew_error_avg"] = ew_avg;

    // ---------- PPS ----------

    // create the lambda adapter for the pps_lin_world function, since it requires additional input arguments
    auto pps_lin_world_adapter = [&](const PossibleWorld &world) {
        return pps_lin_world(world, l_pps, p_s_factor, rng);
    };

    // run the pps algorithm on the input uncertain graph with k
    std::cout << "Running pps with k = " << k_pps << ", l = " << l_pps << " and p_s factor = " << p_s_factor << "..." << std::endl;
    auto [pps_errors, pps_elapsed_time, pps_min, pps_max, pps_avg] = run_method(uncertain_graph, pps_lin_world_adapter, k_pps, exact_centralities_baseline, rng);

    // add the computed errors and the execution time of the current method
    table["pps"] = pps_errors;
    exec_times["pps"] = pps_elapsed_time;

    // update the summary map
    summary_map["k_pps"] = k_pps;
    summary_map["l_pps"] = l_pps;
    summary_map["p_s_factor"] = p_s_factor;
    summary_map["pps_time"] = pps_elapsed_time;
    summary_map["pps_error_min"] = pps_min;
    summary_map["pps_error_max"] = pps_max;
    summary_map["pps_error_avg"] = pps_avg;

    // ---------- SAVE THE RESULTS ----------

    // print some information
    std::cout << "Saving the results..." << std::endl;

    // save the computed centralities in the output file in tsv format
    std::filesystem::path centralities_path = output_dir_path / "errors.tsv";
    save_map_vectors_tsv(centralities_path, table, false);

    // save the execution times of the methods in tsv format
    std::filesystem::path times_path = output_dir_path / "execution_times.tsv";
    save_map_scalars_tsv(times_path, exec_times);

    // save the summary map in tsv format
    std::filesystem::path summary_map_path = output_dir_path / "summary.tsv";
    save_map_scalars_tsv(summary_map_path, summary_map);
}

void multiple_repeated_experiments(
    UncertainGraph &uncertain_graph,
    std::filesystem::path &output_dir_path,
    int n_rep,
    std::vector<int> &n_threads_vec,
    std::vector<int> &k_baseline_vec,
    std::vector<int> &k_mc_vec,
    std::vector<int> &k_ew_vec,
    std::vector<int> &l_ew_vec,
    std::vector<int> &c_ew_vec,
    std::vector<int> &k_pps_vec,
    std::vector<int> &l_pps_vec,
    std::vector<int> &p_s_factor_vec,
    std::mt19937 &rng
) {

    // run an experiment for each possible configuration of parameters
    for (int n_threads : n_threads_vec) {
        for (int k_baseline)
    }

        // set the number of threads for the current experiment
        omp_set_num_threads(n_threads);

        // run n repetitions of the experiment
        for (int i = 0; i < n_rep; ++i) {
            std::filesystem::path output_single_exp = output_dir_path / std::to_string(i);
            run_single_experiment(uncertain_graph, output_single_exp, k_baseline, k_mc, k_ew, l_ew, c_ew, k_pps, l_pps, p_s_factor, rng, i);
        }
}

int main(int argc, char* argv[]) {

    // default command line parameters
    std::filesystem::path input_path;
    std::filesystem::path output_path;
    int k_baseline = 100;
    int k_mc = 10;
    int k_ew = 10;
    int l_ew = 100;
    int c_ew = 5;
    int k_pps = 10;
    int l_pps = 100;
    int p_s_factor = 10;
    int h = 100;
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
        else if (flag == "--k_baseline")
            k_baseline = std::stoi(value);
        else if (flag == "--k_mc")
            k_mc = std::stoi(value);
        else if (flag == "--k_ew")
            k_ew = std::stoi(value);
        else if (flag == "--l_ew")
            l_ew = std::stoi(value);
        else if (flag == "--c_ew")
            c_ew = std::stoi(value);
        else if (flag == "--k_pps")
            k_pps = std::stoi(value);
        else if (flag == "--l_pps")
            l_pps = std::stoi(value);
        else if (flag == "--p_s_factor")
            p_s_factor = std::stoi(value);
        else if (flag == "--h")
            h = std::stoi(value);
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

    // compute an upper bound to the maximum diameter in the uncertain graph
    std::cout << "Computing an upper bound to the diameter of the input uncertain graph..." << std::endl;
    auto start_time_diameter = std::chrono::high_resolution_clock::now();
    int diameter_ub = upper_bound_max_diameter(extract_backbone(uncertain_graph), h, rng);
    auto end_time_diameter = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time_diameter = end_time_diameter - start_time_diameter;
    std::cout << "Estimated upper bound for the diameter: " << diameter_ub << std::endl;
    std::cout << "Time required to estimate the upper bound: " << elapsed_time_diameter.count() << "s" << std::endl;

    // run the experiments
    int exp_rep = 0;
    std::filesystem::path output_single_exp = output_path / std::to_string(exp_rep);
    run_single_experiment(uncertain_graph, output_single_exp, k_baseline, k_mc, k_ew, l_ew, c_ew, k_pps, l_pps, p_s_factor, rng, exp_rep);

    return 0;
}