#include "utils.hpp"
#include "uncertain_graph.hpp"
#include "possible_world.hpp"
#include <iostream>
#include <filesystem>

template <typename T_1, typename T_2>
void save_csv(const std::map<T_1, T_2> &input_map, const std::string &keys_label, const std::string &values_label, const std::filesystem::path &output_path) {

    // create all intermediate directories in case they do not exist yet
    std::filesystem::create_directories(output_path.parent_path());

    // open the output file
    std::ofstream csv_file(output_path);

    // write the header
    csv_file << keys_label << "," << values_label << "\n";

    // write all the pairs in the map
    for (const auto &pair : input_map)
        csv_file << pair.first << "," << pair.second << "\n";
    
    // close the file
    csv_file.close();

}

std::map<int, int> all_distances_in_world(const PossibleWorld &world) {

    // create a map with distances as keys and number of occurrences as values
    std::map<int, int> distance_distr;
    
    // initialize the vectors that will store the results of the bfs from the current node
    std::vector<int> distances(world.n, -1);
    std::vector<int> reached_nodes;
    reached_nodes.reserve(world.n);

    // iterate through each node
    for (int u = 0; u < world.n; ++u) {

        // compute the distances from the current node to all other nodes
        bfs_distances(world, u, distances, reached_nodes);

        // update the distribution of distances with the distances computed by the current bfs
        for (int r : reached_nodes)
            if (r != u)
                distance_distr[distances[r]]++;

        // reset the distances of the reached nodes to the sentinel value -1
        for (int reached : reached_nodes)
            distances[reached] = -1;
    }

    // divide the counts by 2 as we are computing all distances twice
    for (auto &pair : distance_distr)
        pair.second /= 2;
    
    return distance_distr;
}

std::map<int, double> all_distances_in_sampled_worlds(const UncertainGraph &uncertain_graph, int n_samples, std::mt19937 &rng) {

    // create a map with distances as keys and number of occurrences as values
    std::map<int, double> distance_distr;
    
    // initialize the vectors that will store the results of the bfs from the current node in the current world
    std::vector<int> distances(uncertain_graph.n, -1);
    std::vector<int> reached_nodes;
    reached_nodes.reserve(uncertain_graph.n);

    // sample the input number of possible worlds from the input uncertain graph
    for (int i = 0; i < n_samples; ++i) {

        // sample a possible world
        PossibleWorld world = sample_world(uncertain_graph, rng);

        // iterate through each node
        for (int u = 0; u < world.n; ++u) {

            // compute the distances from the current node to all other nodes
            bfs_distances(world, u, distances, reached_nodes);

            // update the distribution of distances with the distances computed by the current bfs
            for (int r : reached_nodes)
                if (r != u)
                    distance_distr[distances[r]]++;

            // reset the distances of the reached nodes to the sentinel value -1
            for (int reached : reached_nodes)
                distances[reached] = -1;
        }
    
    }

    // divide the counts by the input number of sampled possible worlds and by 2, as we are counting distances twice in each sampled possible world
    for (auto &pair : distance_distr)
        pair.second /= (2.0 * n_samples);
    
    return distance_distr;
}

int main(int argc, char* argv[]) {

    // return an error if the number of arguments is not correct
    if (argc != 5) {
        std::cerr << "Error: invalid number of arguments.\n";
        std::cerr << "Usage: " << argv[0] << " <input_path> <output_dir> <n_sampled_worlds> <random_seed>\n";
        return 1;
    }

    // parameters
    double p = 0.5;
    std::mt19937 rng(std::stoi(argv[4]));

    // extract the path to the input file with the dataset
    std::string input_file_path = argv[1];

    // extract the path to the output directory
    std::string output_dir = argv[2];
    std::filesystem::path output_dir_path(output_dir);

    // load the graph
    std::cout << "Loading the graph from the file " << input_file_path << "...\n";
    UncertainGraph uncertain_graph = load_uncertain_graph_no_probs(input_file_path, p);

    // assign uniform probabilities to the edges in the graph
    assign_uniform_edge_probs(uncertain_graph, rng);

    // -------------------- sampled node in sampled world --------------------
    std::cout << "Computing the distribution of distances from a sampled node to all other nodes in a sampled possible world..." << std::endl;

    // sample a possible world
    PossibleWorld world = sample_world(uncertain_graph, rng);

    // sample uniformly at random a node
    std::uniform_int_distribution<int> distr(0, world.n - 1);
    int sampled_node = distr(rng);

    // compute the distances from the sampled node to all other nodes
    std::vector<int> distances(world.n, -1);
    std::vector<int> reached_nodes;
    reached_nodes.reserve(world.n);
    bfs_distances(world, sampled_node, distances, reached_nodes);

    // create a map with distances as keys and number of occurrences as values
    std::map<int, int> node_distance_distr;
    for (int r : reached_nodes)
        if (r != sampled_node)
            node_distance_distr[distances[r]]++;

    // save the map into csv format
    std::filesystem::path node_filename = "sampled_node_sampled_world_distances.csv";
    save_csv(node_distance_distr, "distance", "count", output_dir_path / node_filename);

    // -------------------- all nodes in sampled world --------------------
    std::cout << "Computing the distribution of distances between all nodes in a sampled possible world..." << std::endl;

    // compute and save all distances between all nodes in the sampled possible world
    std::map<int, int> all_nodes_distances_distr = all_distances_in_world(world);
    std::filesystem::path all_nodes_filename = "all_nodes_sampled_world_distances.csv";
    save_csv(all_nodes_distances_distr, "distance", "count", output_dir_path / all_nodes_filename);

    // -------------------- all nodes in several sampled worlds --------------------
    std::cout << "Computing the distribution of distances between all nodes in a set of " << argv[3] << " sampled possible worlds..." << std::endl;

    // compute and save all distances between all nodes in a set of sampled possible worlds
    std::map<int, double> all_nodes_sampled_worlds_distances_distr = all_distances_in_sampled_worlds(uncertain_graph, std::stoi(argv[3]), rng);
    std::filesystem::path all_nodes_sampled_worlds_filename = "all_nodes_set_sampled_worlds_distances.csv";
    save_csv(all_nodes_sampled_worlds_distances_distr, "distance", "count", output_dir_path / all_nodes_sampled_worlds_filename);

    return 0;
}