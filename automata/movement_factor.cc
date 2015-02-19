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
    // We want the baked position, so that new changes to the grid don't
    // influence the ones that follow.
    organism_->GetBakedPosition(&x_, &y_);
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
    organism_->GetBakedPosition(&x_, &y_);
  }

  return y_;
}

double MovementFactor::GetDistance(int x, int y) {
  return pow(pow(x_ - x, 2) + pow(y_ - y, 2), 0.5);
}

}  //  automata
