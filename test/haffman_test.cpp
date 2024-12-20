////
// @file haffman_test.cpp
// @brief
// 测试哈夫曼树
//
// @author yuhaoze
// @email 743544510@qq.com
//
#include <gtest/gtest.h>

#include "haffman.h"
using namespace backup;

TEST(HaffmanTest, CreateHaffmanCode) {
  std::unordered_map<char, uint64_t> mp;
  mp['a'] = 3;
  Haffman haff(mp);
  auto tree = haff.createHaffmanTree();
  auto codes = haff.createHaffmanCode();
  EXPECT_EQ(codes['a'], std::string("0"));

  mp['b'] = 1;
  mp['c'] = 2;
  mp['d'] = 1;
  Haffman haff2(mp);
  tree = haff2.createHaffmanTree();
  codes = haff2.createHaffmanCode();

  EXPECT_EQ(codes['a'], "0");
  EXPECT_EQ(codes['b'], "111");
  EXPECT_EQ(codes['c'], "10");
  EXPECT_EQ(codes['d'], "110");
}