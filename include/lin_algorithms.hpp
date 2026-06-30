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