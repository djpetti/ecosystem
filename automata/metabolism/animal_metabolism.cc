#include <assert.h> // TEMP
#include <stdio.h> // TEMP

#include <cmath>

#include "animal_metabolism.h"

namespace automata {
namespace metabolism {
namespace {

// Energy in fat. (kJ/g)
constexpr double kFatEnergy = 37.0;
// Normalization constants for Kleiber's law. These come from here:
// https://universe-review.ca/R10-35-metabolic.htm
constexpr double kB0 = 14.0149;
constexpr double kB1 = 0.5371;
constexpr double kB2 = 0.0294;
constexpr double kB3 = 4799.0;
// Air density at sea level. (kg/m^3)
constexpr double kAirDensity = 1.225;
// Drag coefficient

// Updates the basal metabolic rate based on the current mass.
// mass: Mass of the organism. (kg)
// body_temp: Body temperature. (K)
// Returns: The basal metabolic rate.
double UpdateBasalRate(double mass, double body_temp) {
  // Calculate default rate from an updated version of Kleiber's law. This
  // comes from a 2010 article in Nature.
  return ::std::pow(10, kB0 + kB1 * ::std::log(mass) +
                            kB2 * ::std::pow(::std::log(mass), 2) -
                            kB3 / body_temp);
}

}  // namespace

AnimalMetabolism::AnimalMetabolism(double mass, double fat_mass,
                                   double body_temp, double scale,
                                   double drag_coefficient)
    : Metabolism(mass),
      body_temp_(body_temp),
      scale_(scale),
      drag_coefficient_(drag_coefficient) {
  // Figure out the initial energy from fat reserves.
  energy_ = fat_mass * 1000 * kFatEnergy * 1000;

  UpdateBasalRate(mass_, body_temp_);
}

void AnimalMetabolism::Consume(const Metabolism *metabolism) {
  // Giving it a negative loss is actually a gain.
  UseEnergy(-metabolism->energy());
}

void AnimalMetabolism::Update(int time) {
  // Calculate energy losses due to basal metabolic rate.
  basal_rate_ = UpdateBasalRate(mass_, body_temp_);
  const double energy_loss = basal_rate_ * time;
  UseEnergy(energy_loss);

  assert(!::std::isnan(energy_) && "Energy is nan.\n");
}

void AnimalMetabolism::UseEnergy(double amount) {
  mass_ -= amount / 1000 / kFatEnergy / 1000;
  energy_ -= amount;
}

void AnimalMetabolism::Move(double distance, int time) {
  // We're going to assume that acceleration and decceleration are negligible,
  // and that most of our energy expendetures are from overcoming friction.
  // Calculate an approximate cross-sectional area based on scale.
  const double area = ::std::pow(scale_, 2);
  // Velocity can be calculated from distance, since we know we are moving it in
  // one iteration.
  const double velocity = distance / time;
  const double drag =
      0.5 * drag_coefficient_ * kAirDensity * area * ::std::pow(velocity, 2);
  // Figure out the work done by drag, which should be equal to the work done by
  // the animal, which should equal the energy expended by the animal.
  const double energy_use = drag * distance;
  UseEnergy(energy_use);
}

void AnimalMetabolism::UpdatePregnancy(int gestation_cycles, int cycle_time,
                                       double birth_mass) {
  // Assuming the mass of the baby increases roughly linearly, calculate the
  // current mass of the baby.
  const double baby_mass_delta = birth_mass / gestation_cycles;
  baby_mass_ += baby_mass_delta;
  // Add the mass of the baby to the total mass of the mother, so metabolism
  // calculations figure correctly.
  mass_ += baby_mass_delta;

  // Calculate energy needs of the baby.
  const double energy_use_rate = UpdateBasalRate(baby_mass_, body_temp_);
  const double baby_energy = energy_use_rate * cycle_time;

  // Subtract this from the energy reserves of the mother.
  UseEnergy(baby_energy);
}

void AnimalMetabolism::Reproduce(double mass) {
  // Subtract the mass from that of the organism.
  mass_ -= mass;
}

double AnimalMetabolism::mass() const {
  return mass_ - baby_mass_;
}

}  // namespace metabolism
}  // namespace automata
