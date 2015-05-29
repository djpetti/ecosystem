#ifndef ECOSYSTEM_AUTOMATA_GRID_H_
#define ECOSYSTEM_AUTOMATA_GRID_H_

#include <vector>

#include "automata/macros.h"
#include "automata/movement_factor.h"

// Defines functions for dealing with the grid at a low level.

// NOTE: Most of the public methods in this class, with the exception of
// Update(), are really intended to be used only by instances of GridObject and
// its derivatives. Assumptions are made elsewhere that this is the case, so if
// you choose to not follow this paradigm, you risk breaking things in an
// unexpected way.

namespace automata {

namespace testing {
// Forward declaration for testing.
class AutomataTest_MotionTest_Test;
class AutomataTest_MotionFactorsTest_Test;
class AutomataTest_OutOfBoundsTest_Test;
}  //  namespace testing

// Forward declaration of GridObject to break circular dependency.
class GridObject;

class Grid {
 public:
  // x_size: Size in the x dimension.
  // y_size: Size in the y dimension.
  Grid(int x_size, int y_size);
  ~Grid();

  // Sets the occupant of a specific cell. nullptr is a valid thing to pass in
  // here. Passing nullptr does not generate conflicts. It will make this cell
  // vacant the next time Update() is called. If any objects are pending
  // insertion, they will override the nullptr. Passing the cell's current
  // occupant does not do anything. If you really want to do that, check out
  // PurgeNew().
  // x: The x coordinate of the cell's location.
  // y: The y coordinate of the cell's location.
  // occupant: The grid object to occupy this cell.
  // Returns: true if the pending object was set correctly, false if there was a
  // conflict, or if you were trying to run this method on a blacklisted cell.
  bool SetOccupant(int x, int y, GridObject *occupant);
  // Clears a cell of its occupant immediately, no updating required. This is
  // necessary so that we can run it when an grid object gets destroyed in order
  // to avoid dead pointers hanging around in the grid, however, its use for
  // everyday operations should be absolutely minimized.
  // x: The x coordinate of the location to purge.
  // y: The y coordinate of the location to purge.
  void ForcePurgeOccupant(int x, int y) {
    Cell *cell = &grid_[x * x_size_ + y];

    if (cell->NewObject == cell->Object) {
      cell->NewObject = nullptr;
    }
    cell->Object = nullptr;
  }
  // x: The x coordinate of the cell's location.
  // y: The y coordinate of the cell's location.
  // Returns: The occupant of the cell, or nullptr if that cell has no occupant.
  GridObject *GetOccupant(int x, int y) {
    return grid_[x * x_size_ + y].Object;
  }
  // Gets any occupant pending insertion at this cell.
  // x: The x coordinate of the cell's location.
  // y: The y coordinate of the cell's location.
  // Returns: The occupant pending insertion at this cell, or nullptr if none
  // is.
  GridObject *GetPending(int x, int y);
  // Gets any occupant currently in the conflicted slot at this cell.
  // x: The x coordinate of the cell's location.
  // y: The y coordinate of the cell's location.
  // Returns: The contents of the cell's conflicted slot.
  GridObject *GetConflict(int x, int y) const {
    return grid_[x * x_size_ + y].ConflictedObject;
  }
  // Clears an object that is pending insertion at this cell. It will not
  // generate conflicts. Will clear anything pending insertion, including
  // nullptr. If object matches the conflicted object instead of the one pending
  // insertion, it will clear the conflicted slot instead.
  // x: The x coordinate of the cell.
  // y: The y coordinate of the cell.
  // object: The object to clear from the cell.
  // Returns: true if it cleared something, false if object did not match
  // anything.
  bool PurgeNew(int x, int y, const GridObject *object);
  // Allows the user to manually set the blacklist status on a cell.
  // x: The x coordinate of the cell.
  // y: The y coordinate of the cell.
  // blacklist: The blacklist status to set.
  void SetBlacklisted(int x, int y, bool blacklist) {
    grid_[x * x_size_ + y].Blacklisted = blacklist;
  }
  // Gets the occupants of the locations in the extended neighborhood around
  // a specific location.
  // x: The x coordinate of the center cell.
  // y: The y coordinate of the center cell.
  // objects: specifies a pointer to a vector of vectors. Each subvector
  // represents the indices in the neighborbood for one level. The subvectors
  // are organized in ascending order by level.
  // levels: If it is something other than one, it includes
  // items in each successive level around the neighborhood.
  // get_new: If true, it gets the organisms in the pending slot. If false, it
  // gets the organisms in the baked slot.
  // Returns: True if it suceeds, false if the center cell is out of bounds
  // of the grid.
  bool GetNeighborhood(int x, int y,
                       ::std::vector< ::std::vector<GridObject *> > *objects,
                       int levels = 1, bool get_new = false);
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
  bool MoveObject(int x, int y, const ::std::vector<MovementFactor> &factors,
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
  // Returns the current scale of the grid.
  double scale() const { return grid_scale_; }
  // Sets the scale of the grid.
  // scale: The length of one side of a grid square.
  void set_scale(double scale) { grid_scale_ = scale; }

 private:
  DISSALOW_COPY_AND_ASSIGN(Grid);

  friend class testing::AutomataTest_MotionTest_Test;
  friend class testing::AutomataTest_MotionFactorsTest_Test;
  friend class testing::AutomataTest_OutOfBoundsTest_Test;

  // A structure for representing cells in the grid.
  struct Cell {
    // The object that is currently occupying the cell.
    GridObject *Object;
    // This object gets filled in to temporarily hold the next occupant of
    // the cell before Update() is run.
    GridObject *NewObject;
    // This object gets filled in if we have a conflict.
    GridObject *ConflictedObject;
    // Whether we want to prevent things from moving here. This flag is mostly
    // meant to be used by things outside the grid to explicitly restrict
    // movement. It is meant to be set for a very limited time period, and gets
    // cleared at the end of every cycle.
    bool Blacklisted;
    // Whether we want to request that this cell keeps its same occupant for the
    // next cycle. Normally, this is just the default and anything else
    // automatically overrides it, but setting this flag makes it conflict
    // instead.
    bool RequestStasis;
  };

  // Calculates the probability of moving to every square in the extended
  // neighborhood.
  // factors: a vector of factors in the grid, which are used to calculate the
  // probabilities.
  // xs: The x coordinates of the locations in the neighborhood.
  // ys: The y coordinates of the locations in the neighborhood.
  // probabilities: an array of probability values. Should be an array capable
  // of holding a number of items equal to the size of the xs and ys vectors.
  void CalculateProbabilities(::std::vector<MovementFactor> &factors,
                              const ::std::vector<int> &xs,
                              const ::std::vector<int> &ys,
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
  bool GetNeighborhoodLocations(int x, int y, ::std::vector<int> *xs,
                                ::std::vector<int> *ys, int levels = 1);
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
  void DoMovement(const double *probabilities, const ::std::vector<int> &xs,
                  const ::std::vector<int> &ys, int *new_x, int *new_y);
  // Looks at factor visibilities and removes any that are not visible to the
  // object.
  // x: The x coordinate of the objects's position.
  // y: The y coordinate of the objects's position.
  // factors: The vector of factors that we will be processing.
  // vision: Maximum distance we can be from a factor in cells, and still
  // perceive it. A negative value means that there is no limit.
  void RemoveInvisible(int x, int y, ::std::vector<MovementFactor> *factors,
                       int vision);
  // Removes any cells for which the Blacklisted attribute is set to true or
  // which are conflicted from consideration for movement.
  // xs: The x coordinates of the cells to consider.
  // ys: The y coordinates of the cells to consider.
  void RemoveUnusable(::std::vector<int> *xs, ::std::vector<int> *ys);

  // Returns whether or not the underlying array is initialized.
  bool IsInitialized() { return initialized_; }

  // Whether or not the underlying array is initialized.
  bool initialized_ = false;
  // The dimensions of the grid.
  int x_size_;
  int y_size_;
  // A pointer to the underlying grid array.
  Cell *grid_;
  // The size of one side of a grid square.
  double grid_scale_ = -1;
};

}  // namespace automata

#endif
