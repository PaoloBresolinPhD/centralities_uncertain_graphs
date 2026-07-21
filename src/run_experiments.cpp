#include "uncertain_graph.hpp"
#include "lin_algorithms.hpp"
#include "harmonic_algorithms.hpp"
#include "utils.hpp"
#include <iostream>
#include <omp.h>
#include <chrono>
#include <filesystem>
#include <utility>
#include <algorithm>
#include <numeric>
#include <variant>

using ConfigType = std::variant<std::string, std::vector<int>, std::vector<double>, int>;

std::tuple<std::vector<double>, double, double, double, double> run_method(const UncertainGraph &uncertain_graph, const std::function<std::vector<double>(const PossibleWorld&, std::mt19937&)> &in_world_centrality_fn, int k, const std::vector<double> &baseline_centralities, std::mt19937 &rng) {

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

void run_single_exact_experiment(const UncertainGraph &uncertain_graph, const std::filesystem::path &output_dir_path, const std::string &centrality, int k, std::mt19937 &rng, int exp_rep, int n_threads) {

    // map that will contain the data to store in the summary tsv
    std::map<std::string, SummaryType> summary_map;
    
    // update the summary map with experiment information
    summary_map["method"] = "mc_baseline";
    summary_map["exp_rep"] = exp_rep;
    summary_map["k"] = k;
    summary_map["n_threads"] = n_threads;

    // set the number of threads to the input value
    omp_set_num_threads(n_threads);

    // initialize the function that computes the exact centrality in a sampled possible world
    auto exact_fn = [centrality](const PossibleWorld &world, std::mt19937 &rng) {
        if (centrality == "lin")
            return exact_lin_world(world);
        return exact_harmonic_world(world);
    };

    // run the exact algorithm
    auto start_time_exact = std::chrono::high_resolution_clock::now();
    std::vector<double> exact_centralities_baseline = mc_centralities_uncertain_graph(uncertain_graph, k, exact_fn, rng);
    auto end_time_exact = std::chrono::high_resolution_clock::now();

    // compute and store the time required to compute the centralities
    std::chrono::duration<double> exact_elapsed_time = end_time_exact - start_time_exact;
    summary_map["time"] = exact_elapsed_time.count();

    // save the computed centralities in the output file in tsv format
    std::cout << "Saving the results..." << std::endl;
    std::filesystem::path centralities_path = output_dir_path / "centralities.tsv";
    std::vector<std::string> col_names = {"node_id", "centrality"};
    save_vector_tsv(centralities_path, exact_centralities_baseline, col_names);

    // save the summary map in tsv format
    std::filesystem::path summary_map_path = output_dir_path / "summary.tsv";
    save_map_scalars_tsv(summary_map_path, summary_map);
}

void run_multiple_repeated_exact_experiments(const UncertainGraph &uncertain_graph, const std::filesystem::path &results_dir_path, const std::string &centrality, const std::vector<int> &k_values, const std::vector<int> &n_threads_values, int n_reps, int random_seed) {

    // run an experiment for each configuration of input parameters
    for (int n_threads : n_threads_values) {
        for (int k : k_values) {
            for (int exp_rep = 0; exp_rep < n_reps; ++exp_rep) {

                // define the random number generator for the current experiment repetition
                std::mt19937 rng(random_seed + exp_rep);

                // print some information about the current experiment
                std::cout << "\nRunning MC baseline with the following parameters:" << std::endl;
                std::cout << "n_threads: " << n_threads << std::endl;
                std::cout << "k: " << k << std::endl;
                std::cout << "exp_rep: " << exp_rep << std::endl;

                // path where to save the results of the current experiment
                std::filesystem::path exp_path = results_dir_path / "mc_baseline" / ("threads_" + std::to_string(n_threads)) / ("k_" + std::to_string(k)) / ("rep_" + std::to_string(exp_rep));

                // create the directories if they do not exist yet
                std::filesystem::create_directories(exp_path);

                // run the experiment
                run_single_exact_experiment(uncertain_graph, exp_path, centrality, k, rng, exp_rep, n_threads);
            }
        }
    }
}

void run_single_mc_experiment(const UncertainGraph &uncertain_graph, const std::filesystem::path &output_dir_path, const std::string &centrality, const std::vector<double> &centralities_baseline, int k, std::mt19937 &rng, int exp_rep, int n_threads) {

    // map that will contain the data to store in the summary tsv
    std::map<std::string, SummaryType> summary_map;
    
    // update the summary map with experiment information
    summary_map["method"] = "mc";
    summary_map["exp_rep"] = exp_rep;
    summary_map["k"] = k;
    summary_map["n_threads"] = n_threads;

    // set the number of threads to the input value
    omp_set_num_threads(n_threads);

    // initialize the function that computes the exact centrality in a sampled possible world
    auto exact_fn = [centrality](const PossibleWorld &world, std::mt19937 &rng) {
        if (centrality == "lin")
            return exact_lin_world(world);
        return exact_harmonic_world(world);
    };

    // run the mc algorithm
    auto [mc_errors, mc_elapsed_time, mc_min, mc_max, mc_avg] = run_method(uncertain_graph, exact_fn, k, centralities_baseline, rng);

    // update the summary map
    summary_map["time"] = mc_elapsed_time;
    summary_map["min_error"] = mc_min;
    summary_map["max_error"] = mc_max;
    summary_map["avg_error"] = mc_avg;

    // save the computed errors in the output file in tsv format
    std::cout << "Saving the results..." << std::endl;
    std::filesystem::path errors_path = output_dir_path / "errors.tsv";
    std::vector<std::string> col_names = {"node_id", "error"};
    save_vector_tsv(errors_path, mc_errors, col_names);

    // save the summary map in tsv format
    std::filesystem::path summary_map_path = output_dir_path / "summary.tsv";
    save_map_scalars_tsv(summary_map_path, summary_map);
}

void run_multiple_repeated_mc_experiments(const UncertainGraph &uncertain_graph, const std::filesystem::path &results_dir_path, const std::string &centrality, int k_baseline, const std::vector<int> &k_values, const std::vector<int> &n_threads_values, int n_reps, int random_seed) {

    // run an experiment for each configuration of input parameters
    for (int n_threads : n_threads_values) {
        for (int k : k_values) {
            for (int exp_rep = 0; exp_rep < n_reps; ++exp_rep) {

                // define the random number generator for the current experiment repetition
                std::mt19937 rng(random_seed + exp_rep);

                // print some information about the current experiment
                std::cout << "\nRunning MC with the following parameters:" << std::endl;
                std::cout << "n_threads: " << n_threads << std::endl;
                std::cout << "k: " << k << std::endl;
                std::cout << "exp_rep: " << exp_rep << std::endl;
                std::cout << "The errors of the estimated centralities are computed with respect to the MC baseline run with the following parameters:" << std::endl;
                std::cout << "n_threads: " << n_threads << std::endl;
                std::cout << "k: " << k_baseline << std::endl;
                std::cout << "exp_rep: " << exp_rep << std::endl;

                // path where to save the results of the current experiment
                std::filesystem::path exp_path = results_dir_path / "mc" / ("threads_" + std::to_string(n_threads)) / ("k_" + std::to_string(k)) / ("rep_" + std::to_string(exp_rep));

                // create the directories if they do not exist yet
                std::filesystem::create_directories(exp_path);

                // path to the centralities computed by the baseline MC, which will be used as reference to compute errors
                std::filesystem::path baseline_centralities_path =
                    results_dir_path /
                    "mc_baseline" /
                    ("threads_" + std::to_string(n_threads)) /
                    ("k_" + std::to_string(k_baseline)) /
                    ("rep_" + std::to_string(exp_rep)) /
                    "centralities.tsv";
                
                // load the baseline centralities
                std::vector<double> baseline_centralities = read_vector_tsv<double>(baseline_centralities_path);

                // run the experiment
                run_single_mc_experiment(uncertain_graph, exp_path, centrality, baseline_centralities, k, rng, exp_rep, n_threads);
            }
        }
    }
}

void run_single_ew_experiment(const UncertainGraph &uncertain_graph, const std::filesystem::path &output_dir_path, const std::string &centrality, const std::vector<double> &centralities_baseline, int k, int l, int c, std::mt19937 &rng, int exp_rep, int n_threads) {

    // map that will contain the data to store in the summary tsv
    std::map<std::string, SummaryType> summary_map;
    
    // update the summary map with experiment information
    summary_map["method"] = "ew";
    summary_map["exp_rep"] = exp_rep;
    summary_map["k"] = k;
    summary_map["l"] = l;
    if (centrality == "lin")
        summary_map["c"] = c;
    summary_map["n_threads"] = n_threads;

    // set the number of threads to the input value
    omp_set_num_threads(n_threads);

    // initialize the function that computes the ew-approximate centralities in a sampled possible world
    auto ew_fn = [centrality, l, c](const PossibleWorld &world, std::mt19937 &rng) {
        if (centrality == "lin")
            return ew_lin_world(world, l, c, rng);
        return ew_harmonic_world(world, l, rng);
    };

    // run the ew algorithm
    auto [ew_errors, ew_elapsed_time, ew_min, ew_max, ew_avg] = run_method(uncertain_graph, ew_fn, k, centralities_baseline, rng);

    // update the summary map
    summary_map["time"] = ew_elapsed_time;
    summary_map["min_error"] = ew_min;
    summary_map["max_error"] = ew_max;
    summary_map["avg_error"] = ew_avg;

    // save the computed errors in the output file in tsv format
    std::cout << "Saving the results..." << std::endl;
    std::filesystem::path errors_path = output_dir_path / "errors.tsv";
    std::vector<std::string> col_names = {"node_id", "error"};
    save_vector_tsv(errors_path, ew_errors, col_names);

    // save the summary map in tsv format
    std::filesystem::path summary_map_path = output_dir_path / "summary.tsv";
    save_map_scalars_tsv(summary_map_path, summary_map);
}

void run_multiple_repeated_ew_experiments(const UncertainGraph &uncertain_graph, const std::filesystem::path &results_dir_path, const std::string &centrality, int k_baseline, const std::vector<int> &k_values, const std::vector<int> l_values, const std::vector<int> c_values, const std::vector<int> &n_threads_values, int n_reps, int random_seed) {

    // run an experiment for each configuration of input parameters
    for (int n_threads : n_threads_values) {
        for (int k : k_values) {
            for (int l : l_values) {
                for (int c : c_values) {
                    for (int exp_rep = 0; exp_rep < n_reps; ++exp_rep) {

                        // define the random number generator for the current experiment repetition
                        std::mt19937 rng(random_seed + exp_rep);

                        // print some information about the current experiment
                        std::cout << "\nRunning EW with the following parameters:" << std::endl;
                        std::cout << "n_threads: " << n_threads << std::endl;
                        std::cout << "k: " << k << std::endl;
                        std::cout << "l: " << l << std::endl;
                        if (centrality == "lin")
                            std::cout << "c: " << c << std::endl;
                        std::cout << "exp_rep: " << exp_rep << std::endl;
                        std::cout << "The errors of the estimated centralities are computed with respect to the MC baseline run with the following parameters:" << std::endl;
                        std::cout << "n_threads: " << n_threads << std::endl;
                        std::cout << "k: " << k_baseline << std::endl;
                        std::cout << "exp_rep: " << exp_rep << std::endl;

                        // path where to save the results of the current experiment
                        std::filesystem::path exp_path = results_dir_path / "ew" / ("threads_" + std::to_string(n_threads)) / ("k_" + std::to_string(k)) / ("l_" + std::to_string(l));
                        if (centrality == "lin")
                            exp_path = exp_path / ("c_" + std::to_string(c));
                        exp_path = exp_path / ("rep_" + std::to_string(exp_rep));

                        // create the directories if they do not exist yet
                        std::filesystem::create_directories(exp_path);

                        // path to the centralities computed by the baseline MC, which will be used as reference to compute errors
                        std::filesystem::path baseline_centralities_path =
                            results_dir_path /
                            "mc_baseline" /
                            ("threads_" + std::to_string(n_threads)) /
                            ("k_" + std::to_string(k_baseline)) /
                            ("rep_" + std::to_string(exp_rep)) /
                            "centralities.tsv";
                        
                        // load the baseline centralities
                        std::vector<double> baseline_centralities = read_vector_tsv<double>(baseline_centralities_path);

                        // run the experiment
                        run_single_ew_experiment(uncertain_graph, exp_path, centrality, baseline_centralities, k, l, c, rng, exp_rep, n_threads);
                    }
                }
            }
        }
    }
}

void run_single_pps_experiment(const UncertainGraph &uncertain_graph, const std::filesystem::path &output_dir_path, const std::string &centrality, const std::vector<double> &centralities_baseline, int k, int l, double delta, std::mt19937 &rng, int exp_rep, int n_threads) {

    // map that will contain the data to store in the summary tsv
    std::map<std::string, SummaryType> summary_map;
    
    // update the summary map with experiment information
    summary_map["method"] = "pps";
    summary_map["exp_rep"] = exp_rep;
    summary_map["k"] = k;
    summary_map["l"] = l;
    summary_map["delta"] = delta;
    summary_map["n_threads"] = n_threads;

    // set the number of threads to the input value
    omp_set_num_threads(n_threads);

    // initialize the function that computes the ew-approximate Lin's index in a sampled possible world
    auto pps_fn = [centrality, k, l, delta](const PossibleWorld &world, std::mt19937 &rng) {
        if (centrality == "lin")
            return pps_lin_world(world, k, l, delta, rng);
        return pps_harmonic_world(world, k, l, delta, rng);
    };

    // run the pps algorithm
    auto [pps_errors, pps_elapsed_time, pps_min, pps_max, pps_avg] = run_method(uncertain_graph, pps_fn, k, centralities_baseline, rng);

    // update the summary map
    summary_map["time"] = pps_elapsed_time;
    summary_map["min_error"] = pps_min;
    summary_map["max_error"] = pps_max;
    summary_map["avg_error"] = pps_avg;

    // save the computed errors in the output file in tsv format
    std::cout << "Saving the results..." << std::endl;
    std::filesystem::path errors_path = output_dir_path / "errors.tsv";
    std::vector<std::string> col_names = {"node_id", "error"};
    save_vector_tsv(errors_path, pps_errors, col_names);

    // save the summary map in tsv format
    std::filesystem::path summary_map_path = output_dir_path / "summary.tsv";
    save_map_scalars_tsv(summary_map_path, summary_map);
}

void run_multiple_repeated_pps_experiments(const UncertainGraph &uncertain_graph, const std::filesystem::path &results_dir_path, const std::string &centrality, int k_baseline, const std::vector<int> &k_values, const std::vector<int> l_values, const std::vector<double> delta_values, const std::vector<int> &n_threads_values, int n_reps, int random_seed) {

    // run an experiment for each configuration of input parameters
    for (int n_threads : n_threads_values) {
        for (int k : k_values) {
            for (int l : l_values) {
                for (double delta : delta_values) {
                    for (int exp_rep = 0; exp_rep < n_reps; ++exp_rep) {

                        // define the random number generator for the current experiment repetition
                        std::mt19937 rng(random_seed + exp_rep);

                        // print some information about the current experiment
                        std::cout << "\nRunning PPS with the following parameters:" << std::endl;
                        std::cout << "n_threads: " << n_threads << std::endl;
                        std::cout << "k: " << k << std::endl;
                        std::cout << "l: " << l << std::endl;
                        std::cout << "delta: " << delta << std::endl;
                        std::cout << "exp_rep: " << exp_rep << std::endl;
                        std::cout << "The errors of the estimated centralities are computed with respect to the MC baseline run with the following parameters:" << std::endl;
                        std::cout << "n_threads: " << n_threads << std::endl;
                        std::cout << "k: " << k_baseline << std::endl;
                        std::cout << "exp_rep: " << exp_rep << std::endl;

                        // path where to save the results of the current experiment
                        std::ostringstream stream_delta;
                        stream_delta << std::fixed << std::setprecision(3) << delta;
                        std::filesystem::path exp_path =
                            results_dir_path /
                                "pps" /
                                ("threads_" + std::to_string(n_threads)) /
                                ("k_" + std::to_string(k)) /
                                ("l_" + std::to_string(l)) /
                                ("delta_" + stream_delta.str()) /
                                ("rep_" + std::to_string(exp_rep));

                        // create the directories if they do not exist yet
                        std::filesystem::create_directories(exp_path);

                        // path to the centralities computed by the baseline MC, which will be used as reference to compute errors
                        std::filesystem::path baseline_centralities_path =
                            results_dir_path /
                            "mc_baseline" /
                            ("threads_" + std::to_string(n_threads)) /
                            ("k_" + std::to_string(k_baseline)) /
                            ("rep_" + std::to_string(exp_rep)) /
                            "centralities.tsv";
                        
                        // load the baseline centralities
                        std::vector<double> baseline_centralities = read_vector_tsv<double>(baseline_centralities_path);

                        // run the experiment
                        run_single_pps_experiment(uncertain_graph, exp_path, centrality, baseline_centralities, k, l, delta, rng, exp_rep, n_threads);
                    }
                }
            }
        }
    }
}

std::map<std::string, ConfigType> parse_config_experiments(const std::filesystem::path &config_path, char delimiter_names, char delimiter_values) {

        // define the map that will be returned
        std::map<std::string, ConfigType> output_map;
        
        // open the config file
        std::ifstream file(config_path);

        // iterate through the lines in the config file
        std::string curr_line;
        while (std::getline(file, curr_line)) {

            // skip empty lines
            if (curr_line.empty())
                continue;

            // separate the name of the parameter from its value
            std::stringstream line_stream(curr_line);
            std::string param_name;
            std::getline(line_stream, param_name, delimiter_names);
            line_stream >> std::ws;
            std::string param_value;
            std::getline(line_stream, param_value);

            // recover from a possible \r character in Windows
            if (!param_value.empty() && param_value.back() == '\r')
                param_value.pop_back();

            // parse the value based on its name
            if (param_name == "input_path" || param_name == "output_path")
                output_map[param_name] = param_value;
            else if (param_name == "exp_reps" || param_name == "random_seed" || param_name == "k_baseline_comparisons")
                output_map[param_name] = std::stoi(param_value);
            else if (param_name == "delta_values")
                output_map[param_name] = parse_string<double>(param_value, delimiter_values);
            else
                output_map[param_name] = parse_string<int>(param_value, delimiter_values);
        }

        return output_map;
}

int main(int argc, char* argv[]) {

    // extract the centrality to compute (either "lin" or "harmonic") and the path to the config file from command line
    if (argc < 3) {
        std::cerr << "Arguments error. Usage: " << argv[0] << " <centrality> <config_file_path>" << std::endl;
        return 1;
    }
    std::string centrality = argv[1];
    if (centrality != "lin" && centrality != "harmonic") {
        std::cerr << "Arguments error. Invalid centrality measure. The available options are \"lin\" or \"harmonic\"" << std::endl;
        return 1;
    }
    std::filesystem::path config_path(argv[2]);

    // character used to separate the name of each config parameter from its value
    char delimiter_names = ':';

    // character used to separate the values in a list of values for a single parameter
    char delimiter_values = ',';

    // load the config parameters
    std::map<std::string, ConfigType> config = parse_config_experiments(config_path, delimiter_names, delimiter_values);
    std::string input_path = std::get<std::string>(config["input_path"]);
    std::string output_path = std::get<std::string>(config["output_path"]);    
    std::vector<int> n_threads_values = std::get<std::vector<int>>(config["n_threads_values"]);
    std::vector<int> k_baseline_values = std::get<std::vector<int>>(config["k_baseline_values"]);
    std::vector<int> k_values = std::get<std::vector<int>>(config["k_values"]);
    std::vector<int> l_values = std::get<std::vector<int>>(config["l_values"]);
    std::vector<int> c_values = {0};    // unused value, needed as flag
    if (centrality == "lin")
        c_values = std::get<std::vector<int>>(config["c_values"]);
    std::vector<double> delta_values = std::get<std::vector<double>>(config["delta_values"]);
    int k_baseline_comparisons = std::get<int>(config["k_baseline_comparisons"]);
    int exp_reps = std::get<int>(config["exp_reps"]);
    int random_seed = std::get<int>(config["random_seed"]);

    // define the random number generator for the initial random operations
    std::mt19937 rng(random_seed);

    // load the input graph
    std::cout << "Loading the graph from the file " << input_path << "..." << std::endl;
    double p = 0.5;
    UncertainGraph uncertain_graph = load_uncertain_graph_no_probs(input_path, p);

    // print some information about the loaded graph
    std::cout << "Number of nodes: " << uncertain_graph.n << std::endl;
    std::cout << "Number of edges: " << uncertain_graph.m << std::endl;

    // assign uniform probabilities to the edges in the graph
    assign_uniform_edge_probs(uncertain_graph, rng);

    // compute an upper bound to the maximum diameter in the uncertain graph
    int h = 20;
    std::cout << "Computing an upper bound to the diameter of the input uncertain graph..." << std::endl;
    auto start_time_diameter = std::chrono::high_resolution_clock::now();
    int diameter_ub = upper_bound_max_diameter(extract_backbone(uncertain_graph), h, rng);
    auto end_time_diameter = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time_diameter = end_time_diameter - start_time_diameter;
    std::cout << "Estimated upper bound for the diameter: " << diameter_ub << std::endl;
    std::cout << "Time required to estimate the upper bound: " << elapsed_time_diameter.count() << "s" << std::endl;

    // run the experiments
    std::cout << "Running experiments on the " << centrality << " centrality." << std::endl;
    run_multiple_repeated_exact_experiments(uncertain_graph, output_path, centrality, k_baseline_values, n_threads_values, exp_reps, random_seed);
    run_multiple_repeated_mc_experiments(uncertain_graph, output_path, centrality, k_baseline_comparisons, k_values, n_threads_values, exp_reps, random_seed);
    run_multiple_repeated_ew_experiments(uncertain_graph, output_path, centrality, k_baseline_comparisons, k_values, l_values, c_values, n_threads_values, exp_reps, random_seed);
    run_multiple_repeated_pps_experiments(uncertain_graph, output_path, centrality, k_baseline_comparisons, k_values, l_values, delta_values, n_threads_values, exp_reps, random_seed);

    return 0;
}
