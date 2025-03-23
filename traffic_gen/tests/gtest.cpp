#include <traffic_gen_common.h>

// Demonstrate some basic assertions.
TEST(HelloTestTrafficGen, BasicAssertions) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("traffic", "gen");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
