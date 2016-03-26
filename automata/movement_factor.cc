#include <math.h>

#include "automata/movement_factor.h"
#include "automata/organism.h"

namespace automata {

MovementFactor::MovementFactor(int x, int y, int strength, int visibility)
    : x_(x), y_(y), strength_(strength), visibility_(visibility) {}

MovementFactor::MovementFactor(Organism *organism, int strength, int visibility)
    : strength_(strength), visibility_(visibility), organism_(organism) {}

bool MovementFactor::SetX(int x) {
  if (organism_) {
    return false;
  }
  x_ = x;
  return true;
}

int MovementFactor::GetX() {
  if (organism_) {
    // Using the non-baked position introduces a dependency on update order, and
    // thus, some inherent randomness. However, it neatly avoids situations like
    // the oscillation problems I was having with mutual attraction.
    organism_->get_position(&x_, &y_);
  }

  return x_;
}

bool MovementFactor::SetY(int y) {
  if (organism_) {
    return false;
  }
  y_ = y;
  return true;
}

int MovementFactor::GetY() {
  if (organism_) {
    organism_->get_position(&x_, &y_);
  }

  return y_;
}

double MovementFactor::GetDistance(int x, int y) {
  return pow(pow(GetX() - x, 2) + pow(GetY() - y, 2), 0.5);
}

}  //  automata
