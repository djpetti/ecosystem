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

// Forward declaration of GridObject to break circular dependency.
class GridObject;

class Grid {
  friend class testing::GridTest;
 public:
  Grid();
  ~Grid();

  // Initializes the grid with the specified parameters.
  // x_size: Size in the x dimension.
  // y_size: Size in the y dimension.
  bool Initialize(int x_size, int y_size);
  // Sets the occupant of a specific cell. nullptr is a valid thing to pass in
  // here, although it's often a better idea to use PurgeNew instead.
  // x: The x coordinate of the cells's location.
  // y: The y coordinate of the cells's location.
  // occupant: The grid object to occupy this cell.
  bool SetOccupant(int x, int y, GridObject *occupant);
  // Clears a cell of its occupant immediately, no updating required. This is
  // necessary so that we can run it when an grid object gets destroyed in order
  // to avoid dead pointers hanging around in the grid, however, its use should
  // be minimized.
  // x: The x coordinate of the location to purge.
  // y: The y coordinate of the location to purge.
  inline void ForcePurgeOccupant(int x, int y) {
    grid_[x * x_size_ + y].Object = nullptr;
  }
  // x: The x coordinate of the cells's location.
  // y: The y coordinate of the cells's location.
  // Returns: The occupant of the cell, or nullptr in case of failure.
  GridObject *GetOccupant(int x, int y);
  // Clears any conflicted object that is pending insertion at this cell.
  // x: The x coordinate of the cell's location.
  // y: The y coordinate of the cell's location.
  inline void PurgeConflict(int x, int y) {
    grid_[x * x_size_ + y].ConflictedObject = nullptr;
  }
  // Clears an object that is pending insertion at this cell.
  // x: The x coordinate of the cell.
  // y: The y coordinate of the cell.
  // object: This is compared to both the conflicted and pending slots in the
  // cell, and the appropriate one will be cleared depending on which matches.
  // Returns: false if none of them matched.
  bool PurgeNew(int x, int y, GridObject *object);
  // Allows the user to manually set the blacklist status on a cell.
  // x: The x coordinate of the cell.
  // y: The y coordinate of the cell.
  // blacklist: The blacklist status to set.
  inline void SetBlacklisted(int x, int y, bool blacklist) {
    grid_[x * x_size_ + y].Blacklisted = blacklist;
  }
  // Returns the occupants of the locations in the extended neighborhood around
  // a specific location. If levels is something other than one, it includes
  // items in each successive level around the neighborhood. The output argument
  // specifies a pointer to a vector of vectors. Each subvector represents the
  // indices in the neighborbood for one level. The subvectors are organized in
  // ascending order by level.
  bool GetNeighborhood(int x, int y,
      ::std::vector<::std::vector<GridObject *> > *objects, int levels = 1);
  // Takes a vector of movement factors, and chooses a location for a grid
  // object to move to.
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
  bool MoveObject(int x, int y,
      const ::std::vector<MovementFactor> & factors,
      int *new_x, int *new_y, int levels = 1, int vision = -1);
  // "Bakes" the state of the grid. Commits any new changes that were made since
  // the last time this was called to the actual grid. Also un-blacklists all
  // cells on the grid.
  // Returns: false if any cell on the grid remains in a conflicted state. All
  // conflicts must be resolved before running this.
  bool Update();
  // Populates two lists with the objects currently involved in conflicts on the
  // grid.
  // objects1: The first set of objects.
  // objects2: The second set of objects. Each object in this vector is
  // conflicted with the object at the same index in objects1.
  void GetConflicted(::std::vector<GridObject *> *objects1,
      ::std::vector<GridObject *> *objects2);

 private:
  // A structure for representing cells in the grid.
  struct Cell {
    // The object that is currently occupying the cell.
    GridObject *Object;
    // This object gets filled in to temporarily hold the next occupant of
    // the cell before Update() is run.
    GridObject *NewObject;
    // This object gets filled in if we have a conflict.
    GridObject *ConflictedObject;
    // Whether we want to prevent things from moving here.
    bool Blacklisted;
  };

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
  // If any locations that should be in the neighborhood are outside the bounds
  // of the grid, they will not be included.
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
  // Takes a set of probabilities, and uses them to calculate where an object
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
  // object.
  // x: The x coordinate of the objects's position.
  // y: The y coordinate of the objects's position.
  // factors: The vector of factors that we will be processing.
  // vision: Maximum distance we can be from a factor in cells, and still
  // perceive it. A negative value means that there is no limit.
  void RemoveInvisible(int x, int y, ::std::vector<MovementFactor> *factors,
      int vision);
  // Removes any cells for which the Blacklisted attribute is set to true from
  // consideration.
  // xs: The x coordinates of the cells to consider.
  // ys: The y coordinates of the cells to consider.
  void RemoveBlacklisted(::std::vector<int> *xs, ::std::vector<int> *ys);

  // Returns whether or not the underlying array is initialized.
  inline bool IsInitialized() {
    return initialized_;
  }

  // Whether or not the underlying array is initialized.
  bool initialized_ = false;
  // The dimensions of the grid.
  int x_size_;
  int y_size_;
  // A pointer to the underlying grid array.
  Cell *grid_;
};

}  // automata

#endif
