#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>

#include "automata/grid.h"

namespace automata {

Grid::Grid(int x_size, int y_size)
    : x_size_(x_size), y_size_(y_size), grid_(new Cell[x_size * y_size]) {
  srand(time(NULL));

  assert(grid_ && "Failed to allocate grid array!\n");

  // Set everything to a default initialization.
  for (int i = 0; i < x_size * y_size; ++i) {
    grid_[i].Object = nullptr;
    grid_[i].NewObject = nullptr;
    grid_[i].ConflictedObject = nullptr;
    grid_[i].Blacklisted = false;
  }
}

Grid::~Grid() { delete[] grid_; }

bool Grid::SetOccupant(int x, int y, GridObject *occupant) {
  Cell *cell = &grid_[x * x_size_ + y];
  if (cell->Blacklisted) {
    if (!occupant || occupant == cell->Object || occupant == cell->NewObject) {
      // We wouldn't do anything anyway in these cases, so this is not a
      // failure.
      return true;
    }
    // We can't really put something on a blacklisted cell.
    return false;
  }

  if (!cell->NewObject || cell->NewObject == cell->Object) {
    // No occupants.
    assert(!cell->ConflictedObject && "Found conflict on vacant cell.");
    cell->NewObject = occupant;
  } else {
    // We have a conflict.
    if (!occupant || occupant == cell->Object || occupant == cell->NewObject) {
      // Setting NewObject to the same thing over again, or to the same thing as
      // Object is not a failure, but doesn't do anything. Same with setting it
      // to nullptr if it's already occupied.
      return true;
    }

    cell->ConflictedObject = occupant;
    // Blacklist this cell so that nothing else moves here.
    cell->Blacklisted = true;
    return false;
  }

  return true;
}

bool Grid::PurgeNew(int x, int y, const GridObject *object) {
  Cell *cell = &grid_[x * x_size_ + y];
  if (object == cell->NewObject) {
    if (cell->ConflictedObject) {
      // Our conflict isn't a conflict anymore.
      cell->NewObject = cell->ConflictedObject;
      cell->ConflictedObject = nullptr;
    } else {
      cell->NewObject = cell->Object;
    }
  } else if (object == cell->ConflictedObject) {
    // Remove conflicted object.
    cell->ConflictedObject = nullptr;
  } else {
    // Could not find object.
    return false;
  }

  return true;
}

GridObject *Grid::GetPending(int x, int y) {
  const Cell *cell = &grid_[x * x_size_ + y];
  if (cell->NewObject == cell->Object) {
    // Technically, there is nothing pending insertion here.
    return nullptr;
  }

  return cell->NewObject;
}

bool Grid::GetNeighborhoodLocations(int x, int y, ::std::vector<int> *xs,
                                    ::std::vector<int> *ys,
                                    int levels /* = 1*/) {
  if (x < 0 || y < 0 || x >= x_size_ || y >= y_size_) {
    // The starting point isn't within the bounds of the grid.
    return false;
  }

  int x_size = 3;
  int y_size = 3;
  for (int level = 1; level <= levels; ++level) {
    const int end_x = x + x_size / 2;
    const int end_y = y + y_size / 2;
    const int start_x = x - x_size / 2;
    const int start_y = y - y_size / 2;

    ::std::vector<int> level_indices;

    // Get the top row and the bottom row.
    for (int i = start_x; i <= end_x; ++i) {
      if (i >= 0 && i < x_size_) {
        if (start_y >= 0) {
          // Point is in-bounds.
          xs->push_back(i);
          ys->push_back(start_y);
        }
        if (end_y < y_size_) {
          xs->push_back(i);
          ys->push_back(end_y);
        }
      }
    }
    // Get the left and right columns, taking into account the corners, which
    // were already accounted for.
    for (int i = start_y + 1; i <= end_y - 1; ++i) {
      if (i >= 0 && i < y_size_) {
        if (start_x >= 0) {
          xs->push_back(start_x);
          ys->push_back(i);
        }
        if (end_x < x_size_) {
          xs->push_back(end_x);
          ys->push_back(i);
        }
      }
    }

    // Compute the new dimensions of the neighborhood.
    x_size += 2;
    y_size += 2;
  }

  return true;
}

bool Grid::GetNeighborhood(
    int x, int y, ::std::vector< ::std::vector<GridObject *> > *objects,
    int levels /* = 1*/) {
  objects->clear();

  ::std::vector<int> xs, ys;
  if (!GetNeighborhoodLocations(x, y, &xs, &ys, levels)) {
    return false;
  }

  // We know how many locations are in each level, so we can divide our results
  // by level.
  uint32_t in_level = 8;
  uint32_t current_i = 0;
  while (current_i < xs.size()) {
    ::std::vector<GridObject *> level_objects;
    for (; current_i < in_level && current_i < xs.size(); ++current_i) {
      level_objects.push_back(GetOccupant(xs[current_i], ys[current_i]));
    }

    // Add the contents of this level to our main output vector.
    objects->push_back(level_objects);

    in_level += 4;
  }

  return true;
}

bool Grid::MoveObject(int x, int y,
                      const ::std::vector<MovementFactor> &factors, int *new_x,
                      int *new_y, int levels /* = 1*/, int vision /* = -1*/) {
  ::std::vector<MovementFactor> visible_factors = factors;
  RemoveInvisible(x, y, &visible_factors, vision);

  ::std::vector<int> xs, ys;
  if (!GetNeighborhoodLocations(x, y, &xs, &ys, levels)) {
    return false;
  }
  // We want it to have the possibility of staying in the same place also.
  xs.push_back(x);
  ys.push_back(y);
  // Remove blacklisted locations from consideration.
  RemoveBlacklisted(&xs, &ys);

  double probabilities[8];
  CalculateProbabilities(visible_factors, xs, ys, probabilities);

  DoMovement(probabilities, xs, ys, new_x, new_y);

  return true;
}

void Grid::CalculateProbabilities(::std::vector<MovementFactor> &factors,
                                  const ::std::vector<int> &xs,
                                  const ::std::vector<int> &ys,
                                  double *probabilities) {
  int total_strength = 0;
  for (auto &factor : factors) {
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
  for (auto &factor : factors) {
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

void Grid::DoMovement(const double *probabilities, const ::std::vector<int> &xs,
                      const ::std::vector<int> &ys, int *new_x, int *new_y) {
  // Get a random float that's somewhere between 0 and 1.
  const double random =
      static_cast<double>(rand()) / static_cast<double>(RAND_MAX);

  // Count up until we're above it.
  double running_total = 0;
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

void Grid::RemoveInvisible(int x, int y, ::std::vector<MovementFactor> *factors,
                           int vision) {
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

void Grid::RemoveBlacklisted(::std::vector<int> *xs, ::std::vector<int> *ys) {
  ::std::vector<int> to_delete;
  for (uint32_t i = 0; i < xs->size(); ++i) {
    if (grid_[(*xs)[i] * x_size_ + (*ys)[i]].Blacklisted) {
      // This cell is blacklisted. Remove it from consideration.
      to_delete.push_back(i);
    }
  }

  for (auto index : to_delete) {
    xs->erase(xs->begin() + index);
    ys->erase(ys->begin() + index);
  }
}

bool Grid::Update() {
  for (int i = 0; i < x_size_ * y_size_; ++i) {
    if (grid_[i].ConflictedObject) {
      // We can't update if we still have unresolved conflicts.
      return false;
    }

    grid_[i].Object = grid_[i].NewObject;
    // Setting them both to be the same by default allows nullptr to be a valid
    // thing to swap in.
    grid_[i].Blacklisted = false;
  }

  return true;
}

void Grid::GetConflicted(::std::vector<GridObject *> *objects1,
                         ::std::vector<GridObject *> *objects2) {
  for (int i = 0; i < x_size_ * y_size_; ++i) {
    if (grid_[i].ConflictedObject) {
      objects1->push_back(grid_[i].NewObject);
      objects2->push_back(grid_[i].ConflictedObject);
    }
  }
}

}  //  automata
