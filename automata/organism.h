#ifndef ECOSYSTEM_AUTOMATA_ORGANISM_H_
#define ECOSYSTEM_AUTOMATA_ORGANISM_H_

#include <stdint.h>
#include <stdio.h>  // TEMP

#include <vector>

#include "automata/grid.h"
#include "automata/grid_object.h"
#include "automata/macros.h"
#include "automata/movement_factor.h"

namespace automata {

// A class for representing an organism. Designed to facilitate handling things
// like grid indices and movement factors.
class Organism : public GridObject {
 public:
  // grid:  The grid that this organism will exist in.
  // index: The organism's index in the Python code.
  Organism(Grid *grid, int index);
  // Set organism's vision.
  // vision: Organism's new vision.
  void set_vision(int vision) { vision_ = vision; }
  // Returns: Organism's vision.
  int get_vision() const { return vision_; }
  // Set organism's speed.
  // speed: Organism's new speed.
  void set_speed(int speed) { speed_ = speed; }
  // Returns: Organism's speed.
  int get_speed() const { return speed_; }
  // Calculates if the organism should move, and where it should move.
  // use_x: Allows user to specify a custom position to calculate movement from.
  // use_y: See use_x.
  // Returns: true if the movement calculations were successful.
  bool UpdatePosition(int use_x = -1, int use_y = -1);
  // Add a new movement factor for this organism.
  // x: The x position of the factor.
  // y: The y position of the factor.
  // strength: The strength of the factor.
  // visibility: How far away the factor can be perceived by this organism, in
  // cells. A negative value means there is no limit.
  inline void AddFactor(int x, int y, int strength, int visibility = -1) {
    MovementFactor factor(x, y, strength, visibility);
    factors_.push_back(factor);
  }
  // Creates a movement factor from an organism, and adds it as a factor to this
  // organism.
  // organism: The organism to turn into a factor.
  // strength: The strength of the factor.
  // visibility: How far away the factor can be perceived by this organism, in
  // cells. A negative value means there is no limit.
  inline void AddFactorFromOrganism(Organism *organism, int strength,
                                    int visibility = -1) {
    MovementFactor factor(organism, strength, visibility);
    factors_.push_back(factor);
    printf("%d: We now have %zu factors.\n", index_, factors_.size());
  }
  const ::std::vector<MovementFactor> &factors() const { return factors_; }
  // Cleans up any references this organism contains to a specified other
  // organism. For now, it only removes movement factors. This is generally
  // called because that organism is being destructed, and all those references
  // are about to become dead pointers.
  // organism: The organism we want to remove references to.
  void CleanupOrganism(const Organism &organism);
  // A default handler for conflicts on the grid between this organism and
  // another. It resolves the conflict by forcing a random one of them to
  // move again. This method can be called on either organism involved in a
  // conflict.
  // Returns: false if it fails to update the position of the organism it is
  // moving, or if it finds that this organism is not conflicted.
  bool DefaultConflictHandler();
  // Specifies that this particular organism has died and is now defunct.
  void Die();
  // Returns: Whether or not the organism is alive.
  inline bool IsAlive() const {
    return alive_;
  }

 private:
  DISSALOW_COPY_AND_ASSIGN(Organism);

  // (Un)blacklists every space in an organism's neighborhood that contains
  // something that would generate a conflict if the organism tried to move
  // there. Conflict handlers will run at the end of a cycle when blacklisting
  // flags are about to get cleared anyway, so it's okay to mess with them.
  // x: The x coordinate of the center of the neighborhood.
  // y: The y coordinate of the center of the neighborhood.
  // blacklist: True if we're blacklisting, false if we're unblacklisting.
  // levels: How many levels to use when calculating the neighborhood.
  void BlacklistOccupied(int x, int y, bool blacklisting, int levels);

  // The set of movement factors on this grid that could possibly affect this
  // organism.
  ::std::vector<MovementFactor> factors_;
  // Maximum distance in cells that the organism can perceive things. Negative
  // means that there is no limit.
  int vision_ = -1;
  // Maximum distance in cells that the organism can move at one time.
  uint32_t speed_ = 1;
  // Whether the organism is alive.
  bool alive_ = true;
};

}  //  automata

#endif
