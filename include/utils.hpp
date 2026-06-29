#pragma once

#include "uncertain_graph.hpp"
#include <string>
#include <random>

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
 * Saves a table of centralities in the input file into tsv format.
 * The produced file will have a column for each vector of centralities and a row for each node.
 * 
 * @param output_path path to the file where to save the centralities.
 * @param centralitites vector of vectors of double. centralities[i] stores the vector of centralities with index i. centralities[i][j] stores the centrality computed by method i for node j.
 * @param column_names vector of strings used to create the header of the tsv file. column_names[i] is the name associated with the vector of centralities centralities[i].
 */
void save_centralities_tsv(const std::string &output_path, const std::vector<std::vector<double>> &centralities, const std::vector<std::string> &column_names);