#ifndef ECOSYSTEM_AUTOMATA_GRID_ITEM_H_
#define ECOSYSTEM_AUTOMATA_GRID_ITEM_H_

#include "automata/grid.h"
#include "automata/macros.h"

namespace automata {

// A simple superclass that represents all objects on the grid.
class GridObject {
 public:
  // grid:  The grid that this object will exist in.
  // index: The object's index in the Python code.
  // x: The x coordinate of the object's position.
  // y: The y coordinate of the object's position.
  GridObject(Grid *grid, int index);
  // Sets the object as pending insertion on the grid.
  // x: The x coordinate of the object's position.
  // y: The y coordinate of the object's position.
  // Returns: True if setting the object's position succeeds, false otherwise.
  bool Initialize(int x, int y) {
    x_ = x;
    y_ = y;
    on_grid_ = true;
    return grid_->SetOccupant(x_, y_, this);
  }
  // Ensures that the grid stores no reference to this object.
  virtual ~GridObject();
  // Set the stored object index.
  // index: The organism's index in the Python code.
  void set_index(int index) { index_ = index; }
  // Returns: The organism's index in the Python code.
  int get_index() const { return index_; };
  // Set the position of the object.
  // x: The x coordinate of the object's position.
  // y: The y coordinate of the object's position.
  bool SetPosition(int x, int y);
  // Get the object's current position, or planned position if the grid hasn't
  // been updated yet.
  // x: Set to the x coordinate.
  // y: Set to the y coordinate.
  void get_position(int *x, int *y) const {
    *x = x_;
    *y = y_;
  }
  // Gets the "true" position of the object. Will return where the object
  // technically still is even if its slated to move somewhere else on the next
  // grid update. If the object is currently not "baked" anywhere, both x and y
  // will be set to -1.
  // x: Set to the x coordinate.
  // y: Set to the y coordinate.
  // Returns: false if the object has never been baked anywhere, true otherwise.
  bool GetBakedPosition(int *x, int *y);
  // Removes the object from the grid. It is okay to call this function more
  // than once.
  bool RemoveFromGrid();
  // Figures out who we're conflicted with.
  // Returns: A pointer to the object we are conflicted with, or nullptr if we
  // are not conflicted with anybody.
  GridObject *GetConflict();

 protected:
  // x and y coordinates of the object's position, present and past, index of
  // the object in the Python code.
  int x_, y_, index_;
  int last_x_ = -1;
  int last_y_ = -1;

  // The grid that this object exists on.
  Grid *grid_;

  // Whether we are on the grid or not.
  bool on_grid_ = false;

 private:
  DISSALOW_COPY_AND_ASSIGN(GridObject);
};

}  //  automata

#endif
