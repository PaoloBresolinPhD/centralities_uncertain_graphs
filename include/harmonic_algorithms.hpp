#pragma once

#include "possible_world.hpp"
#include <vector>
#include <random>

/**
 * Computes the exact harmonic centrality of all nodes in the input possible world.
 * 
 * @param world possible world.
 * 
 * @return vector of double where the component with index v stores the harmonic centrality of v in the input possible world.
 */
std::vector<double> exact_harmonic_world(const PossibleWorld &world);

/**
 * Approximates the harmonic centrality of all nodes in the input possible world using the Eppstein-Wang-based algorithm.
 * 
 * @param world possible world.
 * @param l int representing the number of nodes to sample in the possible world.
 * @param rng random number generator for reproducibility.
 * 
 * @return vector of double where the component with index v stores the harmonic centrality of v in the input possible world.
 */
std::vector<double> ew_harmonic_world(const PossibleWorld &world, int l, std::mt19937 &rng);

/**
 * Extracts a PPS sample of O(l) nodes from the input connected component of the input possible world.
 * 
 * @param world possible world.
 * @param nodes vector with the node ids of the input possible world.
 * @param l integer specifying the dimension O(l) of the PPS sample of nodes to extract.
 * @param p_s double representing the poisson probability to create the initial sample of nodes.
 * @param rng random number generator for reproducibility.
 * 
 * @return map with the sampled nodes and their corresponding sampling probabilities.
 */
std::map<int, double> harmonic_pps_sample(const PossibleWorld &world, const std::vector<int> &nodes, int l, double p_s, std::mt19937 &rng);

/**
 * Approximates the harmonic centrality of all nodes in the input possible world using the PPS-based algorithm.
 * 
 * @param world possible world.
 * @param k int representing the number of possible world to sample. It is required to compute p_s.
 * @param l int suggesting the number of nodes to sample in the possible world. The number of sampled points will be O(l).
 * @param delta double representing the error confidence. It is required to compute p_s.
 * @param rng random number generator for reproducibility.
 * 
 * @return vector of double where the component with index v stores the harmonic centrality of v in the input possible world.
 */
std::vector<double> pps_harmonic_world(const PossibleWorld &world, int k, int l, double delta, std::mt19937 &rng);