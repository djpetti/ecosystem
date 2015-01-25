#include <stdlib.h>
#include <time.h>

#include "automata/organism.h"

namespace automata {

Organism::Organism(Grid *grid, int index, int x, int y) :
    GridObject(grid, index, x, y) {
  srand(time(NULL));
}

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

bool Organism::DefaultConflictHandler(Organism *organism1,
    Organism *organism2) {
  // In this case, we'll pick one of the organisms to move again at random.
  int random = rand() % 2;

  Organism *to_move;
  if (random) {
    to_move = organism1;
  } else {
    to_move = organism2;
  }

  // Before we move, blacklist our current location.
  int x, y;
  to_move->GetPosition(&x, &y);
  grid_->SetBlacklisted(x, y, true);

  if (!to_move->UpdatePosition()) {
    return false;
  }

  grid_->SetBlacklisted(x, y, false);

  return true;
}

} //  automata
