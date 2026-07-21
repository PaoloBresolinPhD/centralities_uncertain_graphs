#pragma once

#include "uncertain_graph.hpp"
#include <string>
#include <random>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <variant>

using SummaryType = std::variant<std::string, int, double>;

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
void save_map_scalars_tsv(const std::filesystem::path &output_path, const std::map<std::string, SummaryType> &map);

/**
 * Saves a vector in tsv format.
 * The indices of the vector are printed as first column.
 * The values of the vector are printed as second column.
 * 
 * @param output_path path to the output file where to save the vector.
 * @param vector vector to save.
 * @param column_names names of the columns to print. The first string is the name of the column with the vector's ids. The second string is the name of the column with the vector values.
 */
template <typename T>
void save_vector_tsv(const std::filesystem::path &output_path, const std::vector<T> &vector, std::vector<std::string> &column_names) {

    // create the output directory if it does not exist
    if (output_path.has_parent_path())
        std::filesystem::create_directories(output_path.parent_path());

    // open the output file
    std::ofstream file(output_path);

    // set the number of digits to save per entry
    file << std::setprecision(10);

    // print the header
    file << column_names[0] << "\t" << column_names[1] << "\n";

    // print the indices and values in the vector
    for (int i = 0; i < (int) vector.size() - 1; ++i)
        file << i << "\t" << vector[i] << "\n";
    file << vector.size() - 1 << "\t" << vector[vector.size() - 1];
}

/**
 * Reads the data from a .tsv file that contains two columns: the first one with ids and the second one with values.
 * The indices are discarded and values are subsequently appended to a vector.
 * The first line is discarded, as it is supposed to contain the header with column names.
 * 
 * @param input_path path to the input .tsv file.
 * 
 * @return vector with the values stored as second column of the .tsv file.
 */
template <typename T>
std::vector<T> read_vector_tsv(const std::filesystem::path &input_path) {

    // initialize the vector that will contain the values stored in the input .tsv file
    std::vector<T> values;

    // open the input file
    std::ifstream file(input_path);

    // discard the first line, which contains the header
    std::string header;
    std::getline(file, header);

    // iterate through lines to load the values stored in the .tsv file
    int id;
    T value;
    while (file >> id >> value)
        values.push_back(value);
    
    return values;
}

/**
 * Parses a string creating a vector with values separated by the input delimiter.
 * 
 * @param input_str string to parse.
 * @param delimiter character that separates values in the string.
 * 
 * @return vector with the values separated by the input delimiter. 
 */
template <typename T>
std::vector<T> parse_string(const std::string &input_str, char delimiter) {

    // define the vector that will contain the extracted values
    std::vector<T> values;

    // define a string stream that will be used to process the string
    std::stringstream str_stream(input_str);

    // iterate through the strings separated by the input delimiter
    std::string value_str;
    while (std::getline(str_stream, value_str, delimiter)) {

        // convert the string to the value type
        std::stringstream converter(value_str);
        T value;
        converter >> value;

        // add the value to the vector
        values.push_back(value);
    }

    return values;
}

/**
 * Samples uniformly at random with replacement l integers from the input vecor.
 * 
 * @param vec vector of int with the values from which sampling.
 * @param l number of values to sample.
 * @param rng random number generator for reproducibility.
 * 
 * @return vector of int with the sampled values.
 */
std::vector<int> uniform_sample_with_replacement(const std::vector<int> &vec, int l, std::mt19937 &rng);

/**
 * Extracts a Poisson sample from the input vector, according to the input probabilities.
 * 
 * @param vec vector of integers with the elements to sample.
 * @param probs vector of doubles with the probabilities of including elements in the Poisson sample.
 * probs[i] is the probability of including vec[i] in the Poisson sample.
 * @param rng random number generator for reproducibility.
 * 
 * @return vector of integers with the sampled nodes.
 */
std::vector<int> poisson_sample(const std::vector<int> &vec, const std::vector<double> &probs, std::mt19937 &rng);

