#include <assert.h>
#include <stdint.h>
#include <stdio.h> // TEMP
#include <stdlib.h>
#include <time.h>

#include <vector>

#include "automata/organism.h"

namespace automata {

Organism::Organism(Grid *grid, int index)
    : GridObject(grid, index) {
  srand(time(NULL));
}

bool Organism::UpdatePosition(int use_x /*= -1*/, int use_y /*= -1*/) {
  int x, y;
  if (use_x < 0 || use_y < 0) {
    use_x = x_;
    use_y = y_;
  }
  // This only returns false if x and y are out of range, so if it is, we have a
  // pretty serious problem.
  printf("%d: Have %zu factors.\n", index_, factors_.size());
  assert(grid_->MoveObject(use_x, use_y, factors_, &x, &y, speed_, vision_) &&
         "MoveObject() failed unexpectedly.");

  if (!SetPosition(x, y)) {
    return false;
  }

  return true;
}

void Organism::BlacklistOccupied(int x, int y, bool blacklisting, int levels) {
  ::std::vector<::std::vector<GridObject *>> in_neighborhood;
  // Once again, this should only fail if we're out of grid bounds.
  printf("Getting neighborhood.\n");
  assert(grid_->GetNeighborhood(x, y, &in_neighborhood,
              levels, true) && "GetNeighborhood() failed unexpectedly.");
  printf("Starting for loop.\n");
  for (auto level : in_neighborhood) {
    for (auto *object : level) {
      int blacklist_x, blacklist_y;
      object->get_position(&blacklist_x, &blacklist_y);
      printf("Blacklisting: (%d, %d).\n", blacklist_x, blacklist_y);
      grid_->SetBlacklisted(blacklist_x, blacklist_y, blacklisting);
    }
  }
}

bool Organism::DefaultConflictHandler() {
  // Get the other organism that we are conflicted with.
  printf("Checking conflict.\n");
  Organism *organism = dynamic_cast<Organism *>(grid_->GetConflict(x_, y_));
  if (!organism) {
    // There's no conflict to resolve.
    return false;
  }
  if (organism == this) {
    // We are running this on the conflicting object instead of the pending one.
    organism = dynamic_cast<Organism *>(grid_->GetPending(x_, y_));
  }

  // In this case, we'll pick one of the organisms to move again at random.
  int random = rand() % 2;

  Organism *to_move;
  if (random) {
    to_move = this;
  } else {
    to_move = organism;
  }

  int baked_x, baked_y;
  printf("Getting baked position.\n");
  to_move->GetBakedPosition(&baked_x, &baked_y);
  bool blacklisted_old = false;
  printf("Checking pending.\n");
  if (grid_->GetPending(baked_x, baked_y)) {
    // We need to blacklist where we came from too.
    grid_->SetBlacklisted(baked_x, baked_y, true);
    blacklisted_old = true;
  }

  // Blacklist anything in the neighborhood that contains something we could
  // conflict with.
  printf("Blacklisting occupied.\n");
  printf("baked: (%d, %d)\n", baked_x, baked_y);
  BlacklistOccupied(baked_x, baked_y, true, to_move->get_speed());

  // Move based on where we were before, so we can't move farther than we should
  // be allowed to in one cycle.
  printf("Updating position.\n");
  if (!to_move->UpdatePosition(baked_x, baked_y)) {
    // This means that our area is so densely populated that we
    // literally can't move anywhere.
    return false;
  }

  printf("Unblacklisting old.\n");
  if (blacklisted_old) {
    grid_->SetBlacklisted(baked_x, baked_y, false);
  }
  // Unblacklist stuff.
  printf("Unblacklisting all.\n");
  BlacklistOccupied(baked_x, baked_y, false, to_move->get_speed());

  return true;
}

void Organism::Die() {
  alive_ = false;
}

void Organism::CleanupOrganism(const Organism &organism) {
  // Check to see if we have any movement factors related to this organism.
  for (uint32_t i = 0; i < factors_.size(); ++i) {
    MovementFactor &factor = factors_[i];
    if (&organism == factor.GetOrganism()) {
      // This one has to go.
      factors_.erase(factors_.begin() + i);
      // Since we lost one, we have to stay on the same index.
      --i;
    }
  }
}

}  //  automata
