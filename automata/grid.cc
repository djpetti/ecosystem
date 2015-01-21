#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>

#include "automata/grid.h"

namespace automata {

Grid::Grid() :
    grid_(nullptr) {
  srand(time(NULL));
}

Grid::~Grid() {
  if (grid_) {
    delete[] grid_;
  }
}

bool Grid::Initialize(int x_size, int y_size) {
  x_size_ = x_size;
  y_size_ = y_size;

  grid_ = new int[x_size * y_size];
  if (!grid_) {
    return false;
  }

  // Set everything to -1, to indicate that no indices are set.
  for (int i = 0; i < x_size_ * y_size_; ++i) {
    grid_[i] = -1;
  }

  initialized_ = true;
  return true;
}

bool Grid::SetIndex(int x, int y, int index) {
  if (!IsInitialized()) {
    return false;
  }

  grid_[x * x_size_ + y] = index;
  return true;
}

int Grid::GetIndex(int x, int y) {
  if (!IsInitialized()) {
    return -1;
  }

  return grid_[x * x_size_ + y];
}

bool Grid::GetNeighborhoodLocations(int x, int y,
    ::std::vector<int> *xs, ::std::vector<int> *ys, int levels/* = 1*/) {
  if (!IsInitialized()) {
    return false;
  }

  int x_size = 3;
  int y_size = 3;
  for (int level = 1; level <= levels; ++level) {
    const int end_x = x + x_size / 2;
    const int end_y = y + y_size / 2;
    const int start_x = x - x_size / 2;
    const int start_y = y - y_size / 2;

    // Ensure that this is within the bounds of the grid.
    if (end_x >= x_size_ || start_x < 0) {
      return false;
    }
    if (end_y >= y_size_ || start_y < 0) {
      return false;
    }

    ::std::vector<int> level_indices;

    // Get the top row and the bottom row.
    for (int i = start_x; i <= end_x; ++i) {
      xs->push_back(i);
      ys->push_back(start_y);
      xs->push_back(i);
      ys->push_back(end_y);
    }
    // Get the left and right columns, taking into account the corners, which
    // were already accounted for.
    for (int i = start_y + 1; i <= end_y - 1; ++i) {
      xs->push_back(start_x);
      ys->push_back(i);
      xs->push_back(end_x);
      ys->push_back(i);
    }

    // Compute the new dimensions of the neighborhood.
    x_size += 2;
    y_size += 2;
  }

  return true;
}

bool Grid::GetNeighborhood(int x, int y,
    ::std::vector<::std::vector<int> > & indices, int levels/* = 1*/) {
  indices.clear();

  ::std::vector<int> xs, ys;
  if (!GetNeighborhoodLocations(x, y, &xs, &ys, levels)) {
    return false;
  }

  // We know how many locations are in each level, so we can divide our results
  // by level.
  uint32_t in_level = 8;
  uint32_t current_i = 0;
  while (current_i < xs.size()) {
    ::std::vector<int> level_indices;
    for (; current_i < in_level; ++current_i) {
      level_indices.push_back(GetIndex(xs[current_i], ys[current_i]));
    }

    // Add the contents of this level to our main output vector.
    indices.push_back(level_indices);

    in_level += 4;
  }

  return true;
}

bool Grid::MoveOrganism(int x, int y,
    const ::std::vector<MovementFactor> & factors,
    int *new_x, int *new_y, int levels/* = 1*/, int vision/* = -1*/) {
  ::std::vector<MovementFactor> visible_factors = factors;
  RemoveInvisible(x, y, &visible_factors, vision);

  ::std::vector<int> xs, ys;
  if (!GetNeighborhoodLocations(x, y, &xs, &ys, levels)) {
    return false;
  }
  // We want it to have the possibility of staying in the same place also.
  xs.push_back(x);
  ys.push_back(y);

  double probabilities[8];
  CalculateProbabilities(visible_factors, xs, ys, probabilities);

  DoMovement(probabilities, xs, ys, new_x, new_y);

  return true;
}

void Grid::CalculateProbabilities(::std::vector<MovementFactor> & factors,
    const ::std::vector<int> & xs, const ::std::vector<int> & ys,
    double *probabilities) {
  int total_strength = 0;
  for (auto & factor : factors) {
    // There is an edge case where all our factors could have a strength of
    // zero.
    total_strength += factor.GetStrength();
  }
  // Having the factor vector empty is valid. It means that there are no
  // factors, and that therefore, there should be an equal probability for every
  // neighborhood location.
  if (factors.empty() || !total_strength) {
    for (uint32_t i = 0; i < xs.size(); ++i) {
      probabilities[i] = 1.0 / xs.size();
    }
    return;
  }

  for (uint32_t i = 0; i < xs.size(); ++i) {
    probabilities[i] = 0;
  }

  // Calculate how far each factor is from each location and use it to change
  // the probabilities.
  for (auto & factor : factors) {
    for (uint32_t i = 0; i < xs.size(); ++i) {
      const double radius = factor.GetDistance(xs[i], ys[i]);

      if (radius != 0) {
        probabilities[i] += (1.0 / radius) * factor.GetStrength();
      } else {
        // If our factor is in the same location that we are.
        probabilities[i] += 10 * factor.GetStrength();
      }
    }
  }

  // Scale probabilities to between 0 and 1.
  double min = 0;
  for (uint32_t i = 0; i < xs.size(); ++i) {
    // First, divide to find the average.
    probabilities[i] /= factors.size();
    // Find the min.
    min = ::std::min(min, probabilities[i]);
  }
  double total = 0;
  for (uint32_t i = 0; i < xs.size(); ++i) {
    // Shift everything to make it positive and calculate total.
    probabilities[i] = (probabilities[i] - min);
    total += probabilities[i];
  }
  for (uint32_t i = 0; i < xs.size(); ++i) {
    // Do the scaling.
    probabilities[i] /= total;
  }
}

void Grid::DoMovement(const double *probabilities,
    const ::std::vector<int> & xs, const ::std::vector<int> & ys,
    int *new_x, int *new_y) {
  // Get a random float that's somewhere between 0 and 1.
  const double random = static_cast<double>(rand()) /
      static_cast<double>(RAND_MAX);

  // Count up until we're above it.
  double running_total;
  for (int i = 0; i < 8; ++i) {
    running_total += probabilities[i];
    if (running_total >= random) {
      *new_x = xs[i];
      *new_y = ys[i];
      return;
    }
  }
  // Floating point weirdness could get us here...
  *new_x = xs[7];
  *new_y = ys[7];
}

void Grid::RemoveInvisible(int x, int y,
    ::std::vector<MovementFactor> *factors, int vision) {
  ::std::vector<uint32_t> to_delete;
  for (uint32_t i = 0; i < factors->size(); ++i) {
    const double radius = (*factors)[i].GetDistance(x, y);

    if (((*factors)[i].GetVisibility() > 0 &&
        radius > (*factors)[i].GetVisibility()) ||
        (vision > 0 && radius > vision)) {
      to_delete.push_back(i);
    }
  }

  for (auto index : to_delete) {
    factors->erase(factors->begin() + index);
  }
}

} //  automata
