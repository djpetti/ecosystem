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

  UpdateBasalRate();
}

void AnimalMetabolism::UpdateBasalRate() {
  // Calculate default rate from an updated version of Kleiber's law. This
  // comes from a 2010 article in nature.
  basal_rate_ = ::std::pow(10, kB0 + kB1 * ::std::log(mass_) +
                                   kB2 * ::std::pow(::std::log(mass_), 2) -
                                   kB3 / body_temp_);
}

void AnimalMetabolism::Consume(const Metabolism *metabolism) {
  // Giving it a negative loss is actually a gain.
  UseEnergy(-metabolism->energy());
}

void AnimalMetabolism::Update(int time) {
  // Calculate energy losses due to basal metabolic rate.
  UpdateBasalRate();
  const double energy_loss = basal_rate_ * time;
  UseEnergy(energy_loss);
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

}  // namespace metabolism
}  // namespace automata
