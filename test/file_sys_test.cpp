#include <gtest/gtest.h>

#include "file_sys.h"
using namespace backup;

TEST(PathTest, PathConstructor) {
  Path p = "/hello/";
  EXPECT_EQ(p.ToString(), "/hello");
  Path q = p;
  EXPECT_EQ(q.ToString(), "/hello");
}

TEST(PathTest, PathJudge) {
  Path p = "hello";
  EXPECT_TRUE(p.IsRelative());
  p = "/hello/h";
  EXPECT_TRUE(!p.IsRelative());
  p = "";
  EXPECT_TRUE(p.IsRelative());
}

TEST(PathTest, SplitPath) {
  Path p = "hello/world";
  std::vector<std::string> path_list = {"hello", "world"};
  EXPECT_EQ(p.SplitPath(), path_list);

  p = "hello";
  path_list = {"hello"};
  EXPECT_EQ(p.SplitPath(), path_list);

  p = "";
  path_list = {};
  EXPECT_EQ(p.SplitPath(), path_list);
}