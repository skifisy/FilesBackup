#include <gtest/gtest.h>

int add(int a, int b) { return a + b; }
// 编写单元测试
TEST(AdditionTest, PositiveNumbers) { EXPECT_EQ(add(3, 4), 7); }

TEST(AdditionTest, NegativeNumbers) { EXPECT_EQ(add(-1, -1), -2); }
