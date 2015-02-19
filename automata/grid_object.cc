#include <assert.h>

#include "grid_object.h"

namespace automata {

GridObject::GridObject(Grid *grid, int index) : index_(index), grid_(grid) {}

GridObject::~GridObject() {
  if (grid_->GetOccupant(x_, y_) != this) {
    // If it hasn't been updated yet, it's relatively easy to get rid of
    // ourselves. Technically, this could return false, especially if we're
    // destructing before we called Initialize(), but there's not a whole lot we
    // can do about that.
    grid_->PurgeNew(x_, y_, this);
  } else {
    grid_->ForcePurgeOccupant(x_, y_);
  }
}

bool GridObject::SetPosition(int x, int y) {
  // Set ourselves at our new location.
  if (!grid_->SetOccupant(x, y, this)) {
    if (grid_->GetConflict(x, y) == this) {
      // We're conflicted, so it's worth updating our current position.
      x_ = x;
      y_ = y;
    }
    return false;
  }

  // We have to remove ourself from our old location on the grid.
  if (grid_->GetOccupant(x_, y_) != this) {
    // The grid hasn't been updated since the last time we set the position.
    assert(grid_->PurgeNew(x_, y_, this) &&
           "PurgeNew() should not return false.");
  } else {
    // The grid has been updated.
    // It's pretty hard for SetOccupant with nullptr to fail...
    assert(grid_->SetOccupant(x_, y_, nullptr) &&
           "SetOccupant() failing on nullptr.");
    last_x_ = x_;
    last_y_ = y_;
  }

  x_ = x;
  y_ = y;

  return true;
}

void GridObject::GetBakedPosition(int *x, int *y) {
  if (grid_->GetOccupant(x_, y_) == this) {
    // The grid has been updated, meaning that our current positions are the
    // baked ones instead of our previous positions.
    *x = x_;
    *y = y_;
  } else {
    // The grid hasn't been updated, meaning that our current positions are
    // as-of-yet unbaked.
    *x = last_x_;
    *y = last_y_;
  }
}

}  //  automata
