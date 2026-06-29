#pragma once

#include "possible_world.hpp"
#include <vector>

/**
 * Computes the exact Lin's index of all nodes in the input possible world.
 * 
 * @param world possible world.
 * 
 * @return vector of double where the component with index v stores the Lin's index of v in the input possible world.
 */
std::vector<double> exact_lin_world(const PossibleWorld &world);