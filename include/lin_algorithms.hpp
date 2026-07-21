#pragma once

#include "possible_world.hpp"
#include <vector>
#include <random>

/**
 * Computes the exact Lin's index of all nodes in the input possible world.
 * 
 * @param world possible world.
 * 
 * @return vector of double where the component with index v stores the Lin's index of v in the input possible world.
 */
std::vector<double> exact_lin_world(const PossibleWorld &world);

/**
 * Approximates the Lin's index of all nodes in the input possible world using the Eppstein-Wang-based algorithm.
 * 
 * @param world possible world.
 * @param l int representing the number of nodes to sample in each connected component of the possible world.
 * @param c double used as threshold for the size of each connected component. If a connected component has less than c nodes, then 0 is returned as centrality for all the nodes in it.
 * @param rng random number generator for reproducibility.
 * 
 * @return vector of double where the component with index v stores the Lin's index of v in the input possible world.
 */
std::vector<double> ew_lin_world(const PossibleWorld &world, int l, double c, std::mt19937 &rng);

/**
 * Extracts a PPS sample of O(l) nodes from the input connected component of the input possible world.
 * 
 * @param world possible world.
 * @param conn_comp_nodes vector of integers with the nodes that are in the connected component from which the nodes have to be sampled.
 * @param l integer specifying the dimension O(l) of the PPS sample of nodes to extract.
 * @param p_s double representing the poisson probability to create the initial sample of nodes.
 * @param rng random number generator for reproducibility.
 * 
 * @return map with the sampled nodes and their corresponding sampling probabilities.
 */
std::map<int, double> lin_pps_sample(const PossibleWorld &world, const std::vector<int> &conn_comp_nodes, int l, double p_s, std::mt19937 &rng);

/**
 * Approximates the Lin's index of all nodes in the input possible world using the PPS-based algorithm.
 * 
 * @param world possible world.
 * @param k int representing the number of possible world to sample. It is required to compute p_s.
 * @param l int suggesting the number of nodes to sample in each connected component of the possible world. The number of sampled points will be O(l).
 * @param delta double representing the error confidence. It is required to compute p_s.
 * @param rng random number generator for reproducibility.
 * 
 * @return vector of double where the component with index v stores the Lin's index of v in the input possible world.
 */
std::vector<double> pps_lin_world(const PossibleWorld &world, int k, int l, double delta, std::mt19937 &rng);
