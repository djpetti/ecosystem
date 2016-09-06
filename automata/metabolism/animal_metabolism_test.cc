#include "gtest/gtest.h"

#include "automata/metabolism/animal_metabolism.h"

namespace automata {
namespace metabolism {

class AnimalMetabolismTest : public ::testing::Test {
 public:
  // Set some reasonable values for the constants.
  AnimalMetabolismTest()
      : metabolism_(kInitialMass, kFatMass, kBodyTemp, kScale,
                    kDragCoefficient) {}

 protected:
  static constexpr double kInitialMass = 0.5;
  static constexpr double kFatMass = 0.1;
  static constexpr double kBodyTemp = 310.15;
  static constexpr double kScale = 0.5;
  static constexpr double kDragCoefficient = 0.37;

  AnimalMetabolism metabolism_;
};

// Does it set reasonable initial values?
TEST_F(AnimalMetabolismTest, InitialValues) {
  // We should have a non-zero initial energy and mass.
  EXPECT_GT(metabolism_.energy(), 0.0);
  EXPECT_EQ(0.5, metabolism_.mass());
}

// Does running Update give us reasonable values?
TEST_F(AnimalMetabolismTest, Update) {
  const double start_energy = metabolism_.energy();
  const double start_mass = metabolism_.mass();
  metabolism_.Update(1);
  const double new_energy = metabolism_.energy();
  const double new_mass = metabolism_.mass();

  // We lose energy just staying alive.
  EXPECT_LT(new_energy, start_energy);
  EXPECT_LT(new_mass, start_mass);

  const double energy_change = new_energy - start_energy;
  const double mass_change = new_mass - kInitialMass;

  metabolism_.Update(5);

  EXPECT_LT(metabolism_.energy(), new_energy);
  EXPECT_LT(metabolism_.mass(), new_mass);
  EXPECT_LT(metabolism_.energy() - new_energy, energy_change);
  EXPECT_LT(metabolism_.mass() - new_mass, mass_change);
}

// Can we use energy the way we expect?
TEST_F(AnimalMetabolismTest, UseEnergy) {
  const double start_energy = metabolism_.energy();

  // Deplete energy completely.
  metabolism_.UseEnergy(start_energy);

  EXPECT_EQ(0.0, metabolism_.energy());
  EXPECT_EQ(kInitialMass - kFatMass, metabolism_.mass());
}

// Does moving around require energy?
TEST_F(AnimalMetabolismTest, MoveTest) {
  const double start_energy = metabolism_.energy();
  const double start_mass = metabolism_.mass();

  // Tell it to move a certain distance.
  metabolism_.Move(1, 1);

  EXPECT_LT(metabolism_.energy(), start_energy);
  EXPECT_LT(metabolism_.mass(), start_mass);
}

// Does consuming other organisms work correctly?
TEST_F(AnimalMetabolismTest, PredationTest) {
  const double start_energy = metabolism_.energy();

  // This is what we're going to eat.
  AnimalMetabolism prey(kInitialMass, kFatMass, kBodyTemp, kScale,
                        kDragCoefficient);

  metabolism_.Consume(&prey);

  // Since the prey has the exact same parameters as we do, we expect the energy
  // to double.
  EXPECT_EQ(start_energy * 2, metabolism_.energy());
  EXPECT_EQ(kInitialMass + kFatMass, metabolism_.mass());
}

// Does the basal rate change as expected?
TEST_F(AnimalMetabolismTest, BasalRateUpdateTest) {
  const double start_energy = metabolism_.energy();

  metabolism_.Update(10);
  const double new_energy = metabolism_.energy();
  const double energy_loss = start_energy - new_energy;

  // Our basal rate should have decreased slighly, so this update should use
  // less energy.
  metabolism_.Update(10);
  EXPECT_LT(new_energy - metabolism_.energy(), energy_loss);
}

// Does reproduction work as expected?
TEST_F(AnimalMetabolismTest, ReproductionTest) {
  const double start_mass = metabolism_.mass();
  const double start_energy = metabolism_.energy();

  // Simulate a pregnancy.
  double last_mass = start_mass;
  double last_energy = start_energy;
  double last_energy_change = 0;
  for (int i = 1; i < 11; ++i) {
    metabolism_.UpdatePregnancy(10, 10, 5);
    EXPECT_LT(metabolism_.mass(), last_mass);
    EXPECT_LT(metabolism_.energy(), last_energy);
    // (These are negative.)
    EXPECT_LT(metabolism_.energy() - last_energy, last_energy_change);

    last_energy_change = metabolism_.energy() - last_energy;
    last_mass = metabolism_.mass();
    last_energy = metabolism_.energy();
  }

  metabolism_.Reproduce(1);
  EXPECT_NEAR(start_mass - 1, metabolism_.mass(), 0.00001);
}

// Can we use all the energy we start out with?
TEST_F(AnimalMetabolismTest, UseAllEnergyTest) {
  double last_energy = metabolism_.energy();

  while (metabolism_.energy() > 0) {
    last_energy = metabolism_.energy();
    metabolism_.Update(10000);

    EXPECT_LT(metabolism_.energy(), last_energy);
  }
}

}  // namespace metabolism
}  // namespace automata
