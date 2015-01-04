#include <vector>

#include "grid.h"
#include "gtest/gtest.h"

namespace automata {
namespace testing {

class GridTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    ASSERT_TRUE(grid_.Initialize(9, 9));
  }

  inline void TestCalculateProbabilities(
      const ::std::vector<MovementFactor> & factors,
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
      ::std::vector<MovementFactor> *factors) {
    grid_.RemoveInvisible(x, y, factors);
  }

  Grid grid_;
};

TEST_F(GridTest, IndexTest) {
  // Do SetIndex() and GetIndex() work?
  EXPECT_LE(grid_.GetIndex(0, 0), 0);
  EXPECT_TRUE(grid_.SetIndex(0, 0, 1));
  EXPECT_EQ(grid_.GetIndex(0, 0), 1);

  // Clear the grid again.
  ASSERT_TRUE(grid_.SetIndex(0, 0, -1));
}

TEST_F(GridTest, NeighborhoodTest) {
  // Does getting the indices in a neighborhood work?
  // Set the extended neighborhood of the location in the middle of the grid to
  // be all ones.
  for (int i = 5; i <= 7; ++i) {
    ASSERT_TRUE(grid_.SetIndex(i, 5, 1));
    ASSERT_TRUE(grid_.SetIndex(i, 7, 1));
  }
  ASSERT_TRUE(grid_.SetIndex(5, 6, 1));
  ASSERT_TRUE(grid_.SetIndex(7, 6, 1));

  ::std::vector<::std::vector<int> > neighborhood;
  EXPECT_TRUE(grid_.GetNeighborhood(6, 6, neighborhood));
  EXPECT_EQ(neighborhood.size(), 1);

  // Add up all the numbers in the neighborhood. If we did this right, we should
  // get exactly six.
  int total = 0;
  for (auto & index : neighborhood[0]) {
    total += index;
  }
  EXPECT_EQ(8, total);
}

TEST_F(GridTest, OutOfBoundsTest) {
  // Does GetNeighborhood fails properly for out-of-bounds input?
  ::std::vector<::std::vector<int> > neighborhood;
  EXPECT_FALSE(grid_.GetNeighborhood(6, 6, neighborhood, 10));
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
  MovementFactor factor {0, 0, 0, -1};
  factors.push_back(factor);
  TestCalculateProbabilities(factors, xs, ys, probabilities);
  for (int i = 1; i < 8; ++i) {
    EXPECT_EQ(probabilities[0], probabilities[i]);
  }

  // An attractive factor in the neighborhood should lead to a high probability
  // for its location.
  factors[0].Strength = 100;
  TestCalculateProbabilities(factors, xs, ys, probabilities);
  for (int i = 1; i < 8; ++i) {
    EXPECT_GT(probabilities[0], probabilities[i]);
  }

  // Two attractive factors in opposite corners of the neighborhood should
  // create two "poles" of attraction.
  factor.X = 2;
  factor.Y = 2;
  factor.Strength = 100;
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
  factors[0].Strength = -100;
  TestCalculateProbabilities(factors, xs, ys, probabilities);
  for (int i = 1; i < 8; ++i) {
    EXPECT_LT(probabilities[0], probabilities[i]);
  }

  // An attractive factor just outside the neighborhood should work similarly to
  // one inside the neighborhood.
  factors[0].X = 3;
  factors[0].Y = 1;
  factors[0].Strength = 100;
  TestCalculateProbabilities(factors, xs, ys, probabilities);
  for (int i = 1; i < 7; ++i) {
    EXPECT_GT(probabilities[7], probabilities[i]);
  }

  // This same attractive factor should stop working if we set its visibility
  // low enough.
  auto invisible_factors = factors;
  invisible_factors[0].Visibility = 1;
  TestRemoveInvisible(1, 1, &invisible_factors);
  EXPECT_TRUE(invisible_factors.empty());
}

} //  testing
} //  automata
