#include <gtest/gtest.h>

int add(int a, int b) { return a + b; }

// 被测试的类
class Calculator {
public:
    int Add(int a, int b) {
        return a + b;
    }

    int Subtract(int a, int b) {
        return a - b;
    }
};

// 编写单元测试
TEST(AdditionTest, PositiveNumbers) { EXPECT_EQ(add(3, 4), 7); }

TEST(AdditionTest, NegativeNumbers) { EXPECT_EQ(add(-1, -1), -2); }

// 参数化测试
class AddTest : public ::testing::TestWithParam<std::tuple<int, int, int>> {};

TEST_P(AddTest, CheckAdd) {
  auto [a, b, expected] = GetParam();
  EXPECT_EQ(add(a, b), expected);
}

// 定义参数
INSTANTIATE_TEST_SUITE_P(AddTestCases, AddTest,
                         ::testing::Values(std::make_tuple(1, 2, 3),
                                           std::make_tuple(5, 7, 12),
                                           std::make_tuple(-1, -1, -2)));


// 测试夹具类
class CalculatorTest : public ::testing::Test {
protected:
    Calculator calculator; // 被测试的 Calculator 实例

    // 在每个测试之前执行
    void SetUp() override {
        // 可以在这里进行一些初始化操作
    }

    // 在每个测试之后执行
    void TearDown() override {
        // 可以在这里进行一些清理操作
    }
};

// 使用测试夹具编写测试用例
TEST_F(CalculatorTest, AddTest) {
    EXPECT_EQ(calculator.Add(2, 3), 5);     // 2 + 3 = 5
    EXPECT_EQ(calculator.Add(-1, 1), 0);    // -1 + 1 = 0
    EXPECT_EQ(calculator.Add(-2, -3), -5);  // -2 + -3 = -5
}

