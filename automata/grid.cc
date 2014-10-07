#include "grid.h"

namespace automata {
namespace grid {

Grid::Grid() :
    grid_(nullptr) {}

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

bool Grid::GetNeighborhood(int x, int y,
    ::std::vector<std::vector<int> > & output, int levels /*= 1*/) {
  if (!IsInitialized()) {
    return false;
  }

  output.clear();

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
      level_indices.push_back(GetIndex(i, start_y));
      level_indices.push_back(GetIndex(i, end_y));
    }
    // Get the left and right columns, taking into account the corners, which
    // were already accounted for.
    for (int i = start_y + 1; i <= end_y - 1; ++i) {
      level_indices.push_back(GetIndex(start_x, i));
      level_indices.push_back(GetIndex(end_x, i));
    }

    // Add our vector of indices to the output level.
    output.push_back(level_indices);

    // Compute the new dimensions of the neighborhood.
    x_size += 2;
    y_size += 2;
  }

  return true;
}

} //  automata
} //  grid
