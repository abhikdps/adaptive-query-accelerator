#include <gtest/gtest.h>
#include "../src/lib.hpp"

TEST(MathTest, Addition) {
    EXPECT_EQ(core::add(2, 2), 4);
    EXPECT_EQ(core::add(-1, 1), 0);
}
