#include "automata/organism.h"

namespace automata {

Organism::Organism(Grid *grid, int index, int x, int y) :
    GridObject(grid, index, x, y) {}

bool Organism::UpdatePosition() {
  int x, y;
  if (!grid_->MoveObject(x_, y_, factors_, &x, &y, speed_, vision_)) {
    return false;
  }

  if (!SetPosition(x, y)) {
    return false;
  }

  return true;
}

} //  automata
