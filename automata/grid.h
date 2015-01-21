#ifndef ECOSYSTEM_AUTOMATA_GRID_H_
#define ECOSYSTEM_AUTOMATA_GRID_H_

#include <vector>

#include "automata/movement_factor.h"

// Defines functions for dealing with the grid at a low level. The way the grid
// works is that the C++ code populates its representation of the grid with
// indices into a list in the Python code, which contains the actual data on
// each grid location.

namespace automata {

namespace testing {
  // Forward declaration for testing.
  class GridTest;
} //  testing

class Grid {
  friend class testing::GridTest;
 public:
  Grid();
  ~Grid();

  // Initializes the grid with the specified parameters.
  // x_size: Size in the x dimension.
  // y_size: Size in the y dimension.
  bool Initialize(int x_size, int y_size);
  // Sets the index of a specific item.
  // x: The x coordinate of the index's location.
  // y: The y coordinate of the index's location.
  // index: The value of the index.
  bool SetIndex(int x, int y, int index);
  // x: The x coordinate of the index's location.
  // y: The y coordinate of the index's location.
  // Returns: The index of a specific item, or -1 in case of failure.
  int GetIndex(int x, int y);
  // Returns the indices of the locations in the extended neighborhood around
  // a specific location. If levels is something other than one, it includes
  // items in each successive level around the neighborhood. The output argument
  // specifies a reference to a vector of vectors. Each subvector represents the
  // indices in the neighborbood for one level. The subvectors are organized in
  // ascending order by level.
  bool GetNeighborhood(int x, int y,
      ::std::vector<::std::vector<int> > & indices, int levels = 1);
  // Takes a vector of movement factors, and chooses a location for an organism
  // to move to.
  // x: x coordinate of the organism's current position.
  // y: y coordinate of the organism's current position.
  // factors: A vector of movement factors that will be considered.
  // new_x: The x coordinate of the organism's new position.
  // new_y: The y coordinate of the organism's new position.
  // levels: The number of levels that will be used when building the
  // neighborhood for this organism, which contains the possible locations where
  // we could move. See GetNeighborhood for an explanation of levels.
  // vision: The maximum number of cells we can be from any factor and still
  // perceive it.
  bool MoveOrganism(int x, int y,
      const ::std::vector<MovementFactor> & factors,
      int *new_x, int *new_y, int levels = 1, int vision = -1);

 private:
  // Calculates the probability of moving to every square in the extended
  // neighborhood.
  // factors: a vector of factors in the grid, which are used to calculate the
  // probabilities.
  // xs: The x coordinates of the locations in the neighborhood.
  // ys: The y coordinates of the locations in the neighborhood.
  // probabilities: an array of probability values. Should be an array capable
  // of holding a number of items equal to the size of the xs and ys vectors.
  void CalculateProbabilities(::std::vector<MovementFactor> & factors,
      const ::std::vector<int> & xs, const ::std::vector<int> & ys,
      double *probabilities);
  // Gets the locations that are in a neighborhood.
  // x: The x coordinate of the location we are finding the neighborhood for.
  // y: The y coordinate of the location we are finding the neighborhood for.
  // xs: Vector to be filled with the x coordinates of the locations in the
  // neighborhood.
  // ys: Vector to be filled with the y coordinates of the locations in the
  // neighborhood.
  // levels: An optional argument that specifies how big the neighborhood will
  // be. A level of 1 includes only the 8 spaces immediately surrounding the
  // location. A level of 2 includes those 8 spaces, and the 16 spaces
  // surrounding them. etc.
  // Returns: true if it succeeds, false if the grid is not initialized, or if
  // the neighborhood would be out of its bounds.
  bool GetNeighborhoodLocations(int x, int y,
      ::std::vector<int> *xs, ::std::vector<int> *ys, int levels = 1);
  // Takes a set of probabilities, and uses them to calculate where an organism
  // should move in its neighborhood.
  // probabilities: The array of probabilities for each location, generally
  // should be the one produced by CalculateProbabilities.
  // xs: The x coordinates of the locations in the neighborhood. Should be the
  // same one as was passed to CalculateProbabilities.
  // ys: The y coordinates of the locations in the neighborhood. Should be the
  // same one as was passed to CalculateProbabilities.
  // new_x: The x coordinate of the organism's new location.
  // new_y: The y coordinate of the organism's new location.
  void DoMovement(const double *probabilities,
      const ::std::vector<int> & xs, const ::std::vector<int> & ys,
      int *new_x, int *new_y);
  // Looks at factor visibilities and removes any that are not visible to the
  // organism.
  // x: The x coordinate of the organism's position.
  // y: The y coordinate of the organism's position.
  // factors: The vector of factors that we will be processing.
  // vision: Maximum distance we can be from a factor in cells, and still
  // perceive it. A negative value means that there is no limit.
  void RemoveInvisible(int x, int y, ::std::vector<MovementFactor> *factors,
      int vision);

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

}  // automata

#endif
