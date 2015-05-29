#ifndef ECOSYSTEM_AUTOMATA_MOVEMENT_FACTOR_H_
#define ECOSYSTEM_AUTOMATA_MOVEMENT_FACTOR_H_

#include "automata/macros.h"

namespace automata {

// Forward declaration to break cyclic dependency.
class Organism;

// A class for representing factors that affect an organism's movement. Movement
// factors are things that change the likelihood that an organism will move to a
// specific location in its neighborhood.
class MovementFactor {
 public:
  // A simple constructor that sets nothing.
  MovementFactor();
  // Constructor lets you set all the parameters right off the bat.
  MovementFactor(int x, int y, int strength, int visibility);
  // Alternate constructor allows you to specify a source organism.
  MovementFactor(Organism *organism, int strength, int visibility);
  // Copy constructor.
  MovementFactor(const MovementFactor &other) = default;
  // Assignment operator.
  MovementFactor &operator=(const MovementFactor &other) = default;

  // Accessors and Mutators for all attributes.
  // The location ones are interesting because they are sourced from our
  // represented organism if we have one. The Mutator actually returns false if
  // this is the case.
  bool SetX(int x);
  int GetX();
  bool SetY(int y);
  int GetY();
  void SetStrength(int strength) { strength_ = strength; }
  int GetStrength() { return strength_; }
  void SetVisibility(int visibility) { visibility_ = visibility; }
  int GetVisibility() { return visibility_; }
  void SetOrganism(Organism *organism) { organism_ = organism; }
  Organism *GetOrgansim() { return organism_; }

  // Returns the distance between this factor and a location. Basically does the
  // distance formula.
  // x: The x coordinate of the location.
  // y: The y coordinate of the location.
  double GetDistance(int x, int y);

 private:
  // The factor's x location on the grid.
  int x_ = 0;
  // The factor's y location on the grid.
  int y_ = 0;
  // The factor's strength. Positive means attractive, negative means repulsive.
  int strength_ = 0;
  // How far away the factor can be perceived from.
  int visibility_ = 0;

  // The organism that this movement factor represents, if it
  // represents one.
  Organism *organism_ = nullptr;
};

}  //  automata

#endif
