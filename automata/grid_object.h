#ifndef ECOSYSTEM_AUTOMATA_GRID_ITEM_H_
#define ECOSYSTEM_AUTOMATA_GRID_ITEM_H_

#include "automata/grid.h"

namespace automata {

// A simple superclass that represents all objects on the grid.
class GridObject {
 public:
  // grid:  The grid that this object will exist in.
  // index: The objects index in the Python code.
  // x: The x coordinate of the object's position.
  // y: The y coordinate of the object's position.
  GridObject(Grid *grid, int index, int x, int y);
  // Ensures that the grid stores no reference to this object.
  virtual ~GridObject();
  // Set the stored object index.
  // index: The organism's index in the Python code.
  inline void SetIndex(int index) {
    index_ = index;
  }
  // Set the position of the object.
  // x: The x coordinate of the object's position.
  // y: The y coordinate of the object's position.
  bool SetPosition(int x, int y);
  // Get the object's current position, or planned position if the grid hasn't
  // been updated yet.
  // x: Set to the x coordinate.
  // y: Set to the y coordinate.
  inline void GetPosition(int *x, int *y) {
    *x = x_;
    *y = y_;
  }
  // Gets the "true" position of the object. Will return where the object
  // technically still is even if its slated to move somewhere else on the next
  // grid update.
  // x: Set to the x coordinate.
  // y: Set to the y coordinate.
  void GetBakedPosition(int *x, int *y);

 protected:
  // x and y coordinates of the object's position, present and past, index of
  // the object in the Python code.
  int x_, y_, last_x_, last_y_, index_;
  // The grid that this object exists on.
  Grid *grid_;
};

} //  automata

#endif
