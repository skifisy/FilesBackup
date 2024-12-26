#include <gtest/gtest.h>

#include "file_meta.h"

using namespace backup;

class DumpTest : public ::testing::Test {
 protected:
  std::ifstream ifs;
  std::ofstream ofs;

  // 在每个测试之前执行
  void SetUp() override {
    ofs.open("temp",
             std::ios::binary | std::ios::trunc);  // 打开用于写入的文件
    ASSERT_TRUE(ofs.is_open());
    ifs.open("temp", std::ios::binary);  // 打开用于读取的文件
    ASSERT_TRUE(ifs.is_open());
  }

  // 在每个测试之后执行
  void TearDown() override {
    ifs.close();
    ofs.close();
    EXPECT_EQ(::system("rm -f temp"), 0);
  }
};

TEST_F(DumpTest, DumpInt) {
  int i = 100;
  int j = -1;

  DumpVar(i, ofs);
  ofs.flush();
  LoadVar(j, ifs);
  EXPECT_EQ(i, j);
}

TEST_F(DumpTest, DumpUint32_t) {
  uint32_t i = 100;
  uint32_t j = 0;

  DumpVar(i, ofs);
  ofs.flush();
  LoadVar(j, ifs);
  EXPECT_EQ(i, j);
}

TEST_F(DumpTest, DumpString) {
  std::string s1 = "hello world";
  std::string s2;
  DumpString(s1, ofs);
  ofs.flush();
  LoadString(s2, ifs);
  EXPECT_EQ(s1, s2);
}

TEST_F(DumpTest, DumpCharArray) {
  char s1[] = "hello";
  std::string s2;
  DumpString(s1, ofs);
  ofs.flush();
  LoadString(s2, ifs);
  EXPECT_EQ(s1, s2);

  char magic[4] = "BKP";
  DumpString(magic, ofs);
  ofs.flush();
  LoadString(s2, ifs);
  EXPECT_EQ(magic, s2);
}

TEST_F(DumpTest, DumpBool) {
  bool b1 = true;
  bool b2 = false;
  DumpVar(b1, ofs);
  ofs.flush();
  LoadVar(b2, ifs);
  EXPECT_EQ(b1, b2);
}