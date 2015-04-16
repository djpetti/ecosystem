#ifndef ECOSYSTEM_AUTOMATA_METABOLISM_METABOLISM_H_
#define ECOSYSTEM_AUTOMATA_METABOLISM_METABOLISM_H_

namespace automata {
namespace metabolism {

// Interface for simulating organism metabolism.
class Metabolism {
 public:
  // mass: The initial total mass of the organism. (kG)
  Metabolism(double mass) : mass_(mass) {}
  virtual ~Metabolism() = default;

  // Calculates change in energy over a given amount of time.
  // time: How much time (in secs).
  virtual void Update(int time) = 0;
  // Subtracts a given amount of energy from the organism to be used.
  // amount: Joules of energy to use.
  virtual void UseEnergy(double amount) = 0;

  // Returns: The current mass of the organism in Kg's.
  double mass() const { return mass_; }
  // Returns: The current energy reserves of the organism in J's.
  double energy() const { return energy_; }

 protected:
  // The mass of the organism in Kg's.
  double mass_;
  // The energy reserves of the organism in Joules.
  double energy_ = 0;
};

}  // automata
}  // metabolism

#endif  // ECOSYSTEM_AUTOMATA_METABOLISM_H_
