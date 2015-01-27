#include "grid_object.h"

namespace automata {

GridObject::GridObject(Grid *grid, int index, int x, int y) :
    x_(x),
    y_(y),
    index_(index),
    grid_(grid) {
  grid_->SetOccupant(x_, y_, this);
}

GridObject::~GridObject() {
  if (grid_->GetOccupant(x_, y_) != this) {
    // If it hasn't been updated yet, it's relatively easy to get rid of
    // ourselves.
    // Technically, PurgeNew can return false, but there's nothing really good
    // to do if it does.
    grid_->PurgeNew(x_, y_, this);
  } else {
    grid_->ForcePurgeOccupant(x_, y_);
  }
}

bool GridObject::SetPosition(int x, int y) {
  // We have to remove ourself from our old location on the grid.
  if (grid_->GetOccupant(x_, y_) != this) {
    // The grid hasn't been updated since the last time we set the position.
    if (!grid_->PurgeNew(x_, y_, this)) {
      return false;
    }
  } else {
    // The grid has been updated.
    grid_->SetOccupant(x_, y_, nullptr);
    last_x_ = x_;
    last_y_ = y_;
  }

  x_ = x;
  y_ = y;
  grid_->SetOccupant(x_, y_, this);

  return true;
}

void GridObject::GetBakedPosition(int *x, int *y) {
  if (grid_->GetOccupant(last_x_, last_y_) != this) {
    // The grid has been updated, meaning that our current positions are the
    // baked ones insted of our previous positions.
    *x = x_;
    *y = y_;
  } else {
    // The grid hasn't been updated, meaning hat our current positions are
    // as-of-yet unbaked.
    *x = last_x_;
    *y = last_y_;
  }
}

} //  automata
