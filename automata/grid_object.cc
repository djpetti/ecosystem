#include "grid_object.h"

namespace automata {

GridObject::GridObject(Grid *grid, int index, int x, int y) :
    x_(x),
    y_(y),
    index_(index),
    grid_(grid) {}

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
    if (!grid_->SetOccupant(x_, y_, nullptr)) {
      return false;
    }
  }

  x_ = x;
  y_ = y;
  return grid_->SetOccupant(x_, y_, this);
}

} //  automata
