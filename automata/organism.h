#ifndef ECOSYSTEM_AUTOMATA_ORGANISM_H_
#define ECOSYSTEM_AUTOMATA_ORGANISM_H_

#include <stdint.h>

#include <vector>

#include "automata/grid.h"
#include "automata/movement_factor.h"

namespace automata {

// A class for representing an organism. Designed to facilitate handling things
// like grid indices and movement factors.
class Organism {
 public:
  // grid:  The grid that this organism will exist in.
  // index: The organism's index in the Python code.
  // x: The x coordinate of the organism's position.
  // y: The y coordinate of the organism's position.
  Organism(Grid *grid, int index, int x, int y);
  // Set the stored organism index.
  // index: The organism's index in the Python code.
  inline void SetIndex(int index) {
    index_ = index;
  }
  // Set organism's vision.
  // vision: Organism's new vision.
  inline void SetVision(int vision) {
    vision_ = vision;
  }
  // Set organism's speed.
  // speed: Organism's new speed.
  inline void SetSpeed(int speed) {
    speed_ = speed;
  }
  // Set the position of the organism.
  // x: The x coordinate of the organism's position.
  // y: The y coordinate of the organism's position.
  inline bool SetPosition(int x, int y) {
    x_ = x;
    y_ = y;
    return grid_->SetIndex(x_, y_, index_);
  }
  // Get the organism's current position.
  // x: Set to the x coordinate.
  // y: Set to the y coordinate.
  inline void GetPosition(int *x, int *y) {
    *x = x_;
    *y = y_;
  }
  // Calculates if the organism should move, and where it should move.
  // Returns: true if the movement calculations were successful.
  bool UpdatePosition();
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
  }

 private:
  // The grid that the organism will exist on.
  Grid *grid_;
  // The organism's index in the Python code.
  int index_;
  // The x coordinate of the organism's position.
  int x_;
  // The y coordinate of the organism's position.
  int y_;
  // The set of movement factors on this grid that could possibly affect this
  // organism.
  ::std::vector<MovementFactor> factors_;
  // Maximum distance in cells that the organism can perceive things. Negative
  // means that there is no limit.
  int vision_ = -1;
  // Maximum distance in cells that the organism can move at one time.
  uint32_t speed_ = 1;
};

} //  automata

#endif
