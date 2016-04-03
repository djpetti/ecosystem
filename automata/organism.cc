#include <assert.h>
#include <stdint.h>
#include <stdio.h>  // TEMP
#include <stdlib.h>
#include <time.h>

#include <list>
#include <vector>

#include "automata/organism.h"

namespace automata {

Organism::Organism(Grid *grid, int index) : GridObject(grid, index) {
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
  if (!grid_->MoveObject(use_x, use_y, factors_, &x, &y, speed_, vision_)) {
    return false;
  }

  if (x_ == x && y_ == y) {
    printf("Still staying in the same place.\n");
  }
  if (!SetPosition(x, y)) {
    return false;
  }

  return true;
}

void Organism::BlacklistOccupied(int x, int y, bool blacklisting, int levels) {
  ::std::vector<::std::vector<GridObject *>> in_neighborhood;
  // Once again, this should only fail if we're out of grid bounds.
  printf("Getting neighborhood.\n");
  assert(grid_->GetNeighborhood(x, y, &in_neighborhood, levels, true) &&
         "GetNeighborhood() failed unexpectedly.");
  printf("Starting for loop.\n");
  for (auto level : in_neighborhood) {
    for (auto *object : level) {
      int blacklist_x, blacklist_y;
      object->get_position(&blacklist_x, &blacklist_y);
      printf("(Un)Blacklisting: (%d, %d).\n", blacklist_x, blacklist_y);
      grid_->SetBlacklisted(blacklist_x, blacklist_y, blacklisting);
    }
  }
}

bool Organism::DefaultConflictHandler(int max_depth) {
  return DoDefaultConflictHandler(0, max_depth);
}

bool Organism::DoDefaultConflictHandler(int current_depth, int max_depth) {
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

  Organism *to_move;
  Organism *other_option;
  if (!current_depth) {
    // In this case, we'll pick one of the organisms to move again at random.
    int random = rand() % 2;

    if (random) {
      to_move = this;
      other_option = organism;
    } else {
      to_move = organism;
      other_option = this;
    }
  } else {
    // If we're recursing, we already moved ourselves, and we don't want to try
    // moving ourselves again.
    to_move = organism;
    other_option = nullptr;
  }

  int baked_x, baked_y;
  printf("Getting baked position.\n");
  if (!to_move->GetBakedPosition(&baked_x, &baked_y)) {
    // If this organism is not baked, move the other one.
    to_move = other_option;
    assert(to_move->GetBakedPosition(&baked_x, &baked_y) &&
           "Resolving conflict would force move of non-baked organism, which "
           "is disallowed.");
  }
  bool blacklisted_old = false;
  printf("Checking pending for (%d, %d)\n", baked_x, baked_y);
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
  const bool updated = to_move->UpdatePosition(baked_x, baked_y);

  printf("Unblacklisting old.\n");
  if (blacklisted_old) {
    grid_->SetBlacklisted(baked_x, baked_y, false);
  }
  // Unblacklist stuff.
  printf("Unblacklisting all.\n");
  BlacklistOccupied(baked_x, baked_y, false, to_move->get_speed());

  if (!updated) {
    // This means that our area is so densely populated that we
    // literally can't move anywhere. Most of the time this shouldn't happen,
    // however, there is one special case where it might. This occurs when a
    // baby is born in a very densely-populated area and has to be placed. In
    // this case, it is safe to recurse until we find a place.
    if (current_depth >= max_depth) {
      // We've reached the limit for how far we can go.
      return false;
    }

    // As long as nothing's blacklisted, UpdatePosition doesn't really care if
    // we generate a conflict or not. It will still try to move the organism to
    // a new place.
    assert(!to_move->UpdatePosition(baked_x, baked_y));
    // Now recurse. What will happen here is the organism we're now conflicting
    // with will innevitably be displaced.
    return to_move->DoDefaultConflictHandler(current_depth + 1, max_depth);
  }

  return true;
}

void Organism::Die() { alive_ = false; }

void Organism::CleanupOrganism(const Organism &organism) {
  // Check to see if we have any movement factors related to this organism.
  auto itr = factors_.begin();
  for (; itr != factors_.end(); ++itr) {
    if (&organism == itr->GetOrganism()) {
      // This one has to go.
      // Post-decrement so that it still points to something valid.
      factors_.erase(itr--);
    }
  }
}

}  //  automata
