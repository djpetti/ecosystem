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

 private:
  // Updates the basal metabolic rate based on the current mass.
  void UpdateBasalRate();

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
