#include <vector>

#include "grid.h"
#include "gtest/gtest.h"

namespace automata {
namespace grid {
namespace testing {

class GridTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    ASSERT_TRUE(grid_.Initialize(9, 9));
  }

  Grid grid_;
};

TEST_F(GridTest, IndexTest) {
  // Tests that SetIndex() and GetIndex() work.
  EXPECT_LE(grid_.GetIndex(0, 0), 0);
  EXPECT_TRUE(grid_.SetIndex(0, 0, 1));
  EXPECT_EQ(grid_.GetIndex(0, 0), 1);

  // Clear the grid again.
  ASSERT_TRUE(grid_.SetIndex(0, 0, -1));
}

TEST_F(GridTest, NeighborhoodTest) {
  // Tests that getting the values in a neighborhood works.
  // Set the extended neighborhood of the location in the middle of the grid to
  // be all ones.
  for (int i = 5; i <= 7; ++i) {
    ASSERT_TRUE(grid_.SetIndex(i, 5, 1));
    ASSERT_TRUE(grid_.SetIndex(i, 7, 1));
  }
  ASSERT_TRUE(grid_.SetIndex(5, 6, 1));
  ASSERT_TRUE(grid_.SetIndex(7, 6, 1));

  ::std::vector<::std::vector<int> > neighborhood;
  grid_.GetNeighborhood(6, 6, neighborhood);
  EXPECT_EQ(neighborhood.size(), 1);

  // Add up all the numbers in the neighborhood. If we did this right, we should
  // get exactly six.
  int total = 0;
  for (auto & index : neighborhood[0]) {
    total += index;
  }
  EXPECT_EQ(8, total);
}

} //  testing
} //  grid
} //  automata
