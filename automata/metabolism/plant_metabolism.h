#ifndef ECOSYSTEM_AUTOMATA_PLANT_METABOLISM_H_
#define ECOSYSTEM_AUTOMATA_PLANT_METABOLISM_H_

#include <random>

#include "automata/metabolism/metabolism.h"

namespace automata {
namespace metabolism {

// Class for simulating plant metabolism.
class PlantMetabolism : public Metabolism {
 public:
  // efficiency: Efficiency of photosynthesis, i.e. what percent of the total
  // energy in the incoming sunlight hitting the leaves gets converted to
  // chemical energy.
  // mass: The initial total mass of the plant. (kG)
  // area_mean: Mean amount of leaf area exposed to sunlight for this plant.
  // (m^2)
  // area_stddev: Standard deviation of leaf area exposed to sunlight for this
  // plant. (m^2)
  // cellulose: Percent of dry biomass that is cellulose.
  // hemicellulose: Percent of dry biomass that is hemicellulose.
  // lignin: Percent of dry biomass that is lignin.
  PlantMetabolism(double mass, double efficiency, double area_mean,
                  double area_stddev, double cellulose, double hemicellulose,
                  double lignin);
  virtual ~PlantMetabolism() = default;

  virtual void Update(int time);
  virtual void UseEnergy(double amount);

 private:
  // Efficiency of photosynthesis.
  const double efficiency_;

  // Normal distribution for picking leaf area.
  ::std::normal_distribution<double> leaf_area_curve_;
  // Number generator for normal distributions.
  ::std::default_random_engine generator_;

  // Percent of dry biomass that is composed of these compounds.
  const double cellulose_;
  const double hemicellulose_;
  const double lignin_;
};

}  // automata
}  // metabolism

#endif  // ECOSYSTEM_AUTOMATA_PLANT_METABOLISM_H_
