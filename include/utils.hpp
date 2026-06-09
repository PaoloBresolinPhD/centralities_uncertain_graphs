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