#ifndef ECOSYSTEM_AUTOMATA_GRID_H_
#define ECOSYSTEM_AUTOMATA_GRID_H_

#include <vector>

// Defines functions for dealing with the grid at a low level. The way the grid
// works is that the C++ code populates its representation of the grid with
// indices into a list in the Python code, which contains the actual data on
// each grid location.

namespace automata {
namespace grid {

class Grid {
 public:
  Grid();
  ~Grid();

  // Initializes the grid with the specified parameters.
  bool Initialize(int x_size, int y_size);
  // Sets the index of a sepcific item.
  bool SetIndex(int x, int y, int index);
  // Returns the index of a specific item, or -1 in case of failure.
  int GetIndex(int x, int y);
  // Returns the indices of the locations in the extended neighborhood around
  // a specific location. If levels is something other than one, it includes
  // items in each successive level around the neighborhood. The output argument
  // specifies a reference to a vector of vectors. Each subvector represents the
  // indices in the neighborbood for one level. The subvectors are organized in
  // ascending order by level.
  bool GetNeighborhood(int x, int y,
      ::std::vector<::std::vector<int> > & output, int levels = 1);

 private:
  // Returns whether or not the underlying array is initialized.
  inline bool IsInitialized() {
    return initialized_;
  }

  // Whether or not the underlying array is initialized.
  bool initialized_;
  // The dimensions of the grid.
  int x_size_;
  int y_size_;
  // A pointer to the underlying grid array.
  int *grid_;
};

}  // grid
}  // automata

#endif
