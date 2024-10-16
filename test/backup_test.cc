#include <gtest/gtest.h>
#include "backup_impl.h"

class MyClassTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // 在每个测试之前执行
        backup = new BackUpImpl();
    }

    void TearDown() override {
        // 在每个测试之后执行
        delete backup;
    }

    BackUp * backup;
};

// 使用测试夹具
// TEST_F(MyClassTestFixture, AddFunction) {
//     EXPECT_EQ(backup->Add(2, 3), 5);
// }

// TEST_F(MyClassTestFixture, SubtractFunction) {
//     EXPECT_EQ(obj->Subtract(10, 5), 5);
// }