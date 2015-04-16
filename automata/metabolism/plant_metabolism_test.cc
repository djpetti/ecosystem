#include "gtest/gtest.h"

#include "automata/metabolism/plant_metabolism.h"

namespace automata {
namespace metabolism {

class PlantMetabolismTest : public ::testing::Test {
 public:
  // Set some reasonable values for the constants.
  PlantMetabolismTest()
      : metabolism_(kInitialMass, 0.02, 0.1, 0.0, kPercentCellulose,
                    kPercentHemicellulose, kPercentLignin) {}

 protected:
  static constexpr double kInitialMass = 0.01;
  static constexpr double kPercentCellulose = 0.4;
  static constexpr double kPercentHemicellulose = 0.3;
  static constexpr double kPercentLignin = 0.2;

  PlantMetabolism metabolism_;
};

// Does it set reasonable initial values?
TEST_F(PlantMetabolismTest, InitialValues) {
  // We should have a non-zero initial energy and mass.
  EXPECT_GT(metabolism_.energy(), 0.0);
  EXPECT_EQ(0.01, metabolism_.mass());
}

// Does running Update give us reasonable values?
TEST_F(PlantMetabolismTest, Update) {
  const double start_energy = metabolism_.energy();
  const double start_mass = metabolism_.mass();
  metabolism_.Update(1);
  const double new_energy = metabolism_.energy();
  const double new_mass = metabolism_.mass();

  EXPECT_GT(new_energy, start_energy);
  EXPECT_GT(new_mass, start_mass);

  const double energy_change = new_energy - start_energy;
  const double mass_change = new_mass - kInitialMass;

  metabolism_.Update(5);

  EXPECT_GT(metabolism_.energy(), new_energy);
  EXPECT_GT(metabolism_.mass(), new_mass);
  EXPECT_GT(metabolism_.energy() - new_energy, energy_change);
  EXPECT_GT(metabolism_.mass() - new_mass, mass_change);
}

// Can we use energy the way we expect?
TEST_F(PlantMetabolismTest, UseEnergy) {
  const double start_energy = metabolism_.energy();

  // Deplete energy completely.
  metabolism_.UseEnergy(start_energy);

  EXPECT_EQ(0.0, metabolism_.energy());
  EXPECT_EQ(kInitialMass *
                (kPercentCellulose + kPercentHemicellulose + kPercentLignin),
            metabolism_.mass());
}

}  // namespace metabolism
}  // namespace automata
