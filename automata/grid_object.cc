#include <assert.h>
#include <stdio.h>  // TEMP

#include "grid_object.h"

namespace automata {

GridObject::GridObject(Grid *grid, int index) : index_(index), grid_(grid) {}

GridObject::~GridObject() {
  // Technically, this can return false, but there's not much to do about it if
  // it does.
  RemoveFromGrid();
}

bool GridObject::RemoveFromGrid() {
  if (on_grid_) {
    if (grid_->GetPending(x_, y_) == this ||
        grid_->GetConflict(x_, y_) == this) {
      // If it hasn't been updated yet, we need to get rid of ourselves at the
      // new location.
      if (!grid_->PurgeNew(x_, y_, this)) {
        return false;
      }
    }
    int baked_x, baked_y;
    GetBakedPosition(&baked_x, &baked_y);
    // Remove ourselves if we're baked somewhere.
    if (grid_->GetOccupant(baked_x, baked_y) == this) {
      grid_->ForcePurgeOccupant(baked_x, baked_y);
    }
  }

  on_grid_ = false;
  return true;
}

bool GridObject::SetPosition(int x, int y) {
  // Edge case: We are setting the same position over again.
  bool request_stasis = false;
  if (x == x_ && y == y_) {
    request_stasis = true;
  }

  // Set ourselves at our new location.
  bool conflicted = false;
  if (!grid_->SetOccupant(x, y, this)) {
    if (grid_->GetConflict(x, y) == this) {
      // We're conflicted.
      conflicted = true;
    } else {
      // We failed for some other reason.
      return false;
    }
  }

  if (request_stasis) {
    // If we're staying in the same place, we're done.
    return true;
  }

  printf("New position: (%d, %d)\n", x, y);
  printf("Old position: (%d, %d)\n", x_, y_);
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

  return !conflicted;
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

GridObject *GridObject::GetConflict() {
  if (grid_->GetPending(x_, y_) == this) {
    return grid_->GetConflict(x_, y_);
  } else if (grid_->GetConflict(x_, y_) == this) {
    return grid_->GetPending(x_, y_);
  } else {
    return nullptr;
  }
}

}  //  automata
