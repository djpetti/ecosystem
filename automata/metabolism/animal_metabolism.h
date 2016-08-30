#ifndef ECOSYSTEM_AUTOMATA_METABOLISM_ANIMAL_METABOLISM_H_
#define ECOSYSTEM_AUTOMATA_METABOLISM_ANIMAL_METABOLISM_H_

#include "automata/metabolism/metabolism.h"

namespace automata {
namespace metabolism {

// Class for simulating animal metabolism.
class AnimalMetabolism : public Metabolism {
 public:
  // mass: The initial total mass of the animal. (kg)
  // fat_mass: The initial mass of the animal's fat reserves. (kg)
  // body_temp: The body temperature of the animal. (K)
  // scale: The scale of the animal. (m)
  // drag_coefficient: The drag coefficient of the animal in air.
  AnimalMetabolism(double mass, double fat_mass, double body_temp, double scale,
                   double drag_coefficient);
  virtual ~AnimalMetabolism() = default;

  virtual void Update(int time);
  virtual void UseEnergy(double amount);

  // Consume another organism, and calculate the nutrient gains by this
  // organism.
  // metabolism: The metabolism object associated with another organism.
  void Consume(const Metabolism *metabolism);
  // Calculates energy loss due to moving.
  // distance: How far we moved. (m)
  // time: Time it took us to move that distance. (s)
  void Move(double distance, int time);
  // This is a special mechanism to simulate a long-term pregnancy. It will
  // cause the parent organism's energy needs to steadily grow as the baby
  // develops.
  // gestation_cycles: How many cycles the gestation period lasts.
  // cycle: The current cycle we are on in the gestation period.
  // cycle_time: The duration of each cycle, in seconds. It is assumed that this
  // method will be called once per cycle.
  // birth_mass: The mass of the baby at birth.
  void UpdatePregnancy(int gestation_cycles, int cycle, int cycle_time,
                       double birth_mass);
  // Allows the animal to reproduce. A given amount of this animal's mass will
  // be transferred into the offspring.
  // offspring_mass: The initial mass of the offspring.
  void Reproduce(double mass);

 private:
  // Basal metabolic rate. (W/kg)
  double basal_rate_;
  // Body temperature of organism. (K)
  double body_temp_;
  // Scale of the organism. (m)
  double scale_;
  // Air drag coefficient of the organism.
  double drag_coefficient_;
};

}  // namespace metabolism
}  // namespace automata

#endif
