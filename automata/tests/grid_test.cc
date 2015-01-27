#include <stdio.h>

#include <vector>

#include "automata/grid.h"
#include "automata/grid_object.h"
#include "automata/movement_factor.h"
#include "gtest/gtest.h"

namespace automata {
namespace testing {

class GridTest : public ::testing::Test {
 protected:
  GridTest() : grid_(9, 9) {}

  inline void TestCalculateProbabilities(
      ::std::vector<MovementFactor> & factors,
      const ::std::vector<int> & xs, const ::std::vector<int> & ys,
      double *probabilities) {
    grid_.CalculateProbabilities(factors, xs, ys, probabilities);
  }

  inline void TestDoMovement(const double *probabilities,
      const ::std::vector<int> & xs, const ::std::vector<int> & ys,
      int *new_x, int *new_y) {
    grid_.DoMovement(probabilities, xs, ys, new_x, new_y);
  }

  inline void TestGetNeighborhoodLocations(int x, int y,
      ::std::vector<int> *xs, ::std::vector<int> *ys, int levels = 1) {
    EXPECT_TRUE(grid_.GetNeighborhoodLocations(x, y, xs, ys, levels));
  }

  inline void TestRemoveInvisible(int x, int y,
      ::std::vector<MovementFactor> *factors, int vision) {
    grid_.RemoveInvisible(x, y, factors, vision);
  }

  Grid grid_;
};

TEST_F(GridTest, OccupantTest) {
  // Do SetOccupant() and GetOccupant() work?
  EXPECT_EQ(grid_.GetOccupant(0, 0), nullptr);

  GridObject object(&grid_, 0, 0, 0);
  EXPECT_TRUE(grid_.Update());
  EXPECT_EQ(grid_.GetOccupant(0, 0), &object);

  // Clear the grid again.
  grid_.SetOccupant(0, 0, nullptr);
  ASSERT_TRUE(grid_.Update());
}

TEST_F(GridTest, NeighborhoodTest) {
  // Does getting the indices in a neighborhood work?
  // Set the extended neighborhood of the location in the middle of the grid to
  // be all ones.
  GridObject object(&grid_, 0, 0, 0);
  for (int i = 5; i <= 7; ++i) {
    grid_.SetOccupant(i, 5, &object);
    grid_.SetOccupant(i, 7, &object);
  }
  grid_.SetOccupant(5, 6, &object);
  grid_.SetOccupant(7, 6, &object);
  ASSERT_TRUE(grid_.Update());

  ::std::vector<::std::vector<GridObject *> > neighborhood;
  EXPECT_TRUE(grid_.GetNeighborhood(6, 6, &neighborhood));
  EXPECT_EQ(1, neighborhood.size());

  // Check that everything in the neighborhood is what we expect it to be.
  for (auto new_object : neighborhood[0]) {
    EXPECT_EQ(&object, new_object);
  }
}

TEST_F(GridTest, OutOfBoundsTest) {
  // Does GetNeighborhood deal properly with out-of-bounds input?
  ::std::vector<::std::vector<GridObject *> > neighborhood;
  // Giving it a starting point outside the boundaries of the grid should make
  // it fail.
  EXPECT_FALSE(grid_.GetNeighborhood(-1, -1, &neighborhood));
  // Putting it in a corner should truncate the neighborhood.
  EXPECT_TRUE(grid_.GetNeighborhood(0, 0, &neighborhood));
  EXPECT_EQ(3, neighborhood[0].size());
}

TEST_F(GridTest, MotionTest) {
  // Does DoMovement make a reasonable choice given the array of probabilities?
  // Make a probabilities array with only one possible choice.
  double probabilities[8];
  probabilities[0] = 1;
  for (int i = 1; i < 8; ++i) {
    probabilities[i] = 0;
  }

  // Use GetNeighborhoodLocations to generate xs and ys vectors.
  ::std::vector<int> xs, ys;
  TestGetNeighborhoodLocations(1, 1, &xs, &ys);

  int new_x, new_y;
  TestDoMovement(probabilities, xs, ys, &new_x, &new_y);
  EXPECT_EQ(xs[0], new_x);
  EXPECT_EQ(ys[0], new_y);
}

TEST_F(GridTest, MotionFactorsTest) {
  // Do movement factors influence probabilities the way we would expect?
  ::std::vector<MovementFactor> factors;
  double probabilities[8];
  ::std::vector<int> xs, ys;
  TestGetNeighborhoodLocations(1, 1, &xs, &ys);

  // No factors should lead to equal probability for every location.
  TestCalculateProbabilities(factors, xs, ys, probabilities);
  for (int i = 1; i < 8; ++i) {
    EXPECT_EQ(probabilities[0], probabilities[i]);
  }

  // A factor with a strength of zero should have the same effect.
  MovementFactor factor(0, 0, 0, -1);
  factors.push_back(factor);
  TestCalculateProbabilities(factors, xs, ys, probabilities);
  for (int i = 1; i < 8; ++i) {
    EXPECT_EQ(probabilities[0], probabilities[i]);
  }

  // An attractive factor in the neighborhood should lead to a high probability
  // for its location.
  factors[0].SetStrength(100);
  TestCalculateProbabilities(factors, xs, ys, probabilities);
  for (int i = 1; i < 8; ++i) {
    EXPECT_GT(probabilities[0], probabilities[i]);
  }

  // Two attractive factors in opposite corners of the neighborhood should
  // create two "poles" of attraction.
  factor.SetX(2);
  factor.SetY(2);
  factor.SetStrength(100);
  factors.push_back(factor);
  TestCalculateProbabilities(factors, xs, ys, probabilities);
  // The two poles.
  EXPECT_EQ(probabilities[5], probabilities[0]);
  for (int i = 1; i < 8; ++i) {
    if (i != 5) {
      EXPECT_GT(probabilities[0], probabilities[i]);
    }
  }

  // A repulsive factor in the neighborhood should do the opposite.
  factors.pop_back();
  factors[0].SetStrength(-100);
  TestCalculateProbabilities(factors, xs, ys, probabilities);
  for (int i = 1; i < 8; ++i) {
    EXPECT_LT(probabilities[0], probabilities[i]);
  }

  // An attractive factor just outside the neighborhood should work similarly to
  // one inside the neighborhood.
  factors[0].SetX(3);
  factors[0].SetY(1);
  factors[0].SetStrength(100);
  TestCalculateProbabilities(factors, xs, ys, probabilities);
  for (int i = 1; i < 7; ++i) {
    EXPECT_GT(probabilities[7], probabilities[i]);
  }

  // This same attractive factor should stop working if we set its visibility
  // low enough.
  auto invisible_factors = factors;
  invisible_factors[0].SetVisibility(1);
  TestRemoveInvisible(1, 1, &invisible_factors, -1);
  EXPECT_TRUE(invisible_factors.empty());

  // We should also get this same result if we set the organism's vision low
  // enough.
  TestRemoveInvisible(1, 1, &factors, 1);
  EXPECT_TRUE(factors.empty());
}

} //  testing
} //  automata
