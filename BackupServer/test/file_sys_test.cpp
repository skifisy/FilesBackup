#include <gtest/gtest.h>

#include <unordered_set>
#include "file_sys.h"
using namespace backup;

TEST(PathTest, PathConstructor)
{
    Path p = "/hello/";
    EXPECT_EQ(p.ToString(), "/hello");
    Path q = p;
    EXPECT_EQ(q.ToString(), "/hello");
}

TEST(PathTest, PathJudge)
{
    Path p = "hello";
    EXPECT_TRUE(p.IsRelative());
    p = "/hello/h";
    EXPECT_TRUE(!p.IsRelative());
    p = "";
    EXPECT_TRUE(p.IsRelative());
}

TEST(PathTest, SplitPath)
{
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

TEST(PathTest, GetFileLinkTo)
{
    EXPECT_EQ(::system("rm -f f2"), 0);
    EXPECT_EQ(::system("ln -s ./dir1/f1 f2"), 0);
    Path p = GetFileLinkTo("f2");
    EXPECT_EQ(p.ToString(), "./dir1/f1");
    EXPECT_EQ(::system("rm -f f2"), 0);
}

TEST(PathTest, FullPath)
{
    Path p = "./dir/f";
    Path fullpath = p.ToFullPath();
    EXPECT_EQ(fullpath.ToString(), (GetCurPath() / p).ToString());
}

TEST(PathTest, ParentPath)
{
    Path p = "./dir/f/";
    Path parent = p.ParentPath();
    EXPECT_EQ(parent.ToString(), "./dir");
    p = "dir";
    parent = p.ParentPath();
    EXPECT_EQ(parent.ToString(), "");

    p = "";
    parent = p.ParentPath();
    EXPECT_EQ(parent.ToString(), "");
}

TEST(PathTest, GetFilesFromDir)
{
    EXPECT_EQ(::system("mkdir dir && touch dir/f1 dir/f2 dir/f3"), 0);
    std::unordered_set<std::string> st;
    st.insert("f1");
    st.insert("f2");
    st.insert("f3");
    for (auto &path : GetFilesFromDir("dir")) {
        EXPECT_EQ(st.contains(path.ToString()), true);
    }
    EXPECT_EQ(::system("rm -rf dir"), 0);
}

TEST(PathTest, RemoveFile)
{
    EXPECT_TRUE(::system("touch f1") == 0);
    EXPECT_TRUE(::system("mkdir dir && touch dir/f1 dir/f2") == 0);
    EXPECT_TRUE(RemoveFile(std::string("f1")));
    EXPECT_TRUE(RemoveAll(std::string("dir")));
    EXPECT_FALSE(::system("test -e f1") == 0);
    EXPECT_FALSE(::system("test -e dir") == 0);
}