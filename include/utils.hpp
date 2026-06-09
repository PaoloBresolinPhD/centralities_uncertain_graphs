#pragma once

#include "uncertain_graph.hpp"
#include <string>

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
UncertainGraph load_uncertain_graph_no_probs(std::string file_path, double p);