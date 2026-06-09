#pragma once

#include <vector>
#include <random>
#include "uncertain_graph.hpp"

/**
 * Samples a possible world from the input uncertain graph.
 *
 * @param uncertain_graph input uncertain graph.
 * @param rng random number generator for reproducibility.
 *
 * @return possible world sampled from the input uncertain graph.
*/
PossibleWorld sample_world(const UncertainGraph &uncertain_graph, std::mt19937 &rng);