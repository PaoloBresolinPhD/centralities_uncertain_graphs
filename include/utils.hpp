#pragma once

#include "uncertain_graph.hpp"
#include <string>
#include <random>
#include <filesystem>
#include <fstream>
#include <iterator>

/**
 * Loads a deterministic graph from file into an UncertainGraph with default edge probabilities p.
 * Once an edge (u, v) is found, also the edge (v, u) is added, since we consider undirected graphs.
 * We discard self-loops (u, u).
 * If the same edge is encountered multiple times, then it it stored just once.
 * This also means that if we found (u, v) and we then find (v, u), we do not add (v, u) again.
 * 
 * @param file_path path to the file with edges stored in different lines as space-separated integers.
 * @param p default probability to assign to the edges.
 * 
 * @return UncertainGraph with the loaded edges.
 */
UncertainGraph load_uncertain_graph_no_probs(const std::string &file_path, double p);

/**
 * Maps each string identifying a node to a consecutive integer id.
 * Both the converted dataset and the mapping that associates each integer id to the corresponding string id are saved.
 * 
 * @param input_dataset_path path to the file with the input dataset, where each node is encoded as a string.
 * @param output_dataset_path path to the file where to store the dataset with the nodes converted into integer ids.
 * @param output_map_path path to the file where to store the mapping that associates each integer id back to the corresponding string.
 */
void assign_node_ids(const std::string &input_dataset_path, const std::string &output_dataset_path, const std::string &output_map_path);

/**
 * Assigns a probabilibity to each edge, according to a uniform distribution.
 * 
 * @param uncertain_graph UncertainGraphs to be modified.
 * @param rng random number generator for reproducibility.
 */
void assign_uniform_edge_probs(UncertainGraph &uncertain_graph, std::mt19937 &rng);

/**
 * Saves a map with string keys and vector values in tsv format.
 * The produced file will have a column for each key and a row for each entry in the vector.
 * The keys will be printed in the first row to identify the printed columns.
 * 
 * @param output_path path to the file where to save the map.
 * @param table_map map with strings as key and vectors as values.
 * @param add_node_column if true, a column with consecutive node ids is printed as first column. Otherwise, only the map is printed.
 */
template <typename T>
void save_map_vectors_tsv(const std::filesystem::path &output_path, const std::map<std::string, std::vector<T>> &table_map, bool add_node_column) {

    // create the output directory if it does not exist
    if (output_path.has_parent_path())
        std::filesystem::create_directories(output_path.parent_path());

    // open the output file
    std::ofstream file(output_path);

    // set the number of digits to save per entry
    file << std::setprecision(10);

    // print the header
    if (add_node_column)
        file << "Node\t";
    auto last_col_it = std::prev(table_map.end());
    for (auto it = table_map.begin(); it != last_col_it; ++it)
        file << it->first << "\t";
    file << last_col_it->first << "\n";

    // iterate through columns and rows to fill the file
    for (int i = 0; i < (int) table_map.begin()->second.size(); ++i) {
        if (add_node_column)
            file << i << "\t";
        for (auto it = table_map.begin(); it != last_col_it; ++it)
            file << it->second[i] << "\t";
        file << last_col_it->second[i] << "\n";
    }
}

/**
 * Saves a map of scalars in tsv format.
 * The keys of the map are printed as names of the columns.
 * 
 * @param output_path path to the output file where to save the map.
 * @param map map to save.
 */
template <typename T>
void save_map_scalars_tsv(const std::filesystem::path &output_path, const std::map<std::string, T> &map) {

    // create the output directory if it does not exist
    if (output_path.has_parent_path())
        std::filesystem::create_directories(output_path.parent_path());

    // open the output file
    std::ofstream file(output_path);

    // set the number of digits to save per entry
    file << std::setprecision(10);

    // print the header
    auto last_val_it = std::prev(map.end());
    for (auto it = map.begin(); it != last_val_it; ++it)
        file << it->first << "\t";
    file << last_val_it->first << "\n";

    // print the values in the vector
    for (auto it = map.begin(); it != last_val_it; ++it)
        file << it->second << "\t";
    file << last_val_it->second << "\n";
}
