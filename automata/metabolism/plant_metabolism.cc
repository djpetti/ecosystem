#include <time.h>

#include "automata/metabolism/plant_metabolism.h"

namespace automata {
namespace metabolism {
namespace {

// Average sunlight intensity on earth's surface. (W/m^2) This number comes
// from here: http://en.wikipedia.org/wiki/Sunlight
constexpr double kSolarEnergy = 1120;
// Gibbs free energy per mol of CO2 for photosynthesis.
// (Joules)
constexpr double kPhotosynthesisDeltaG = 114.0 * 4184;
// Gibbs free energy for mol of glucose for respiration.
constexpr double kRespirationDeltaG = -2880000;
// Molecular mass of glucose. (g/mol)
constexpr double kGlucoseMolecularMass = 180.16;

}  // namespace

PlantMetabolism::PlantMetabolism(double mass, double efficiency,
                                 double area_mean, double area_stddev,
                                 double cellulose, double hemicellulose,
                                 double lignin)
    : Metabolism(mass),
      efficiency_(efficiency),
      leaf_area_curve_(area_mean, area_stddev),
      generator_(time(NULL)),
      cellulose_(cellulose),
      hemicellulose_(hemicellulose),
      lignin_(lignin) {
  // Figure out how much energy we start with.
  const double max_energy =
      (mass_ * 1000) / kGlucoseMolecularMass * -kRespirationDeltaG;
  energy_ = max_energy * (1 - (cellulose_ + hemicellulose_ + lignin_));
}

void PlantMetabolism::Update(int time) {
  // Assuming a normal distribution, extract a value for the leaf area
  // exposed
  // to light.
  const double leaf_area = leaf_area_curve_(generator_);

  // Calculate the power of the plant, in watts.
  const double power = leaf_area * kSolarEnergy * efficiency_;
  // Calculate how much energy we produced in this time, in Joules.
  double energy_gain = power * time;

  // To calculate the mass gain, we figure out how many moles of glucose we
  // produced.
  // The basic equation is this: 6C02 + 6H2O --> C6H12O6 + 6O2
  const double mols_glucose = energy_gain / kPhotosynthesisDeltaG;
  const double grams_glucose = mols_glucose * kGlucoseMolecularMass;
  const double mass_gain = grams_glucose / 1000.0;

  // We'll assume that we can't free up energy from cellulose,
  // hemicellulose, or
  // lignin reserves, so that decreases our total energy.
  energy_gain *= (1 - (cellulose_ + hemicellulose_ + lignin_));

  energy_ += energy_gain;
  mass_ += mass_gain;
}

void PlantMetabolism::UseEnergy(double amount) {
  // Figure out how much glucose we'd need to metabolize. (Assume anything
  // more
  // sophisticated has metabolic pathways that release an equivalent amount
  // of
  // energy.)
  const double mols_required = amount / -kRespirationDeltaG;
  const double kg_required = mols_required * kGlucoseMolecularMass / 1000.0;

  mass_ -= kg_required;
  energy_ -= amount;
}

}  // metabolism
}  // automata
