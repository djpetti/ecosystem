#include "automata/organism.h"

namespace automata {

Organism::Organism(Grid *grid, int index, int x, int y) :
    grid_(grid),
    index_(index),
    x_(x),
    y_(y) {}

bool Organism::UpdatePosition() {
  int x, y;
  if (!grid_->MoveOrganism(x_, y_, factors_, &x, &y, speed_, vision_)) {
    return false;
  }

  if (!SetPosition(x, y)) {
    return false;
  }

  return true;
}

} //  automata
