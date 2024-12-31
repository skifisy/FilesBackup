#include <gtest/gtest.h>
#include "backup_impl.h"
#include <unordered_set>
using namespace backup;
class BackupTestFixture : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // 在每个测试之前执行
        backup = new BackUpImpl();
    }

    void TearDown() override
    {
        // 在每个测试之后执行
        delete backup;
    }

    BackUp *backup;
};

// 使用测试夹具
TEST_F(BackupTestFixture, BackupBatch)
{
    EXPECT_EQ(::system("mkdir dir && touch dir/f1 dir/f2"), 0);
    std::vector<std::string> filelist;
    filelist.emplace_back("dir");
    filelist.emplace_back("dir/f1");
    filelist.emplace_back("dir/f2");
    BackupConfig config{"backup_test", ".", false, ""};

    backup->BackupBatch(config, filelist);
    auto [status, root] = backup->GetFileList("backup_test");
    EXPECT_EQ(status.code, OK);
    EXPECT_EQ(::system("rm -rf dir backup_test"), 0);
    auto &node = root->children_;
    EXPECT_TRUE(node.contains("dir"));
    EXPECT_TRUE(node.contains("f1"));
    EXPECT_TRUE(node.contains("f2"));
}

TEST_F(BackupTestFixture, BackupBatch2)
{
    EXPECT_EQ(::system("mkdir dir && touch dir/f1 dir/f2"), 0);
    std::vector<std::pair<std::string, std::string>> filelist;
    filelist.emplace_back("dir", "");
    filelist.emplace_back("dir/f1", "dir");
    filelist.emplace_back("dir/f2", "dir");
    BackupConfig config{"backup_test", ".", false, ""};

    backup->BackupBatch(config, filelist);
    auto [status, root] = backup->GetFileList("backup_test");
    EXPECT_EQ(status.code, OK);
    EXPECT_EQ(::system("rm -rf dir backup_test"), 0);
    auto &node = root->children_;
    EXPECT_TRUE(node.contains("dir"));
    auto &node1 = node["dir"]->children_;
    EXPECT_TRUE(node1.contains("f1"));
    EXPECT_TRUE(node1.contains("f2"));
}

TEST_F(BackupTestFixture, BackupBatchEncrypted)
{
    EXPECT_EQ(::system("mkdir dir && touch dir/f1 dir/f2"), 0);
    std::vector<std::pair<std::string, std::string>> filelist;
    filelist.emplace_back("dir", "");
    filelist.emplace_back("dir/f1", "dir");
    filelist.emplace_back("dir/f2", "dir");
    BackupConfig config{"backup_test", ".", true, "hello"};

    backup->BackupBatch(config, filelist);
    auto [s, encrypted] = backup->isEncrypted("backup_test");
    EXPECT_EQ(s.code, OK);
    EXPECT_EQ(encrypted, true);

    auto [status, root] = backup->GetFileList("backup_test", "hello");
    EXPECT_EQ(status.code, OK);
    EXPECT_EQ(::system("rm -rf dir backup_test"), 0);
    auto &node = root->children_;
    EXPECT_TRUE(node.contains("dir"));
    auto &node1 = node["dir"]->children_;
    EXPECT_TRUE(node1.contains("f1"));
    EXPECT_TRUE(node1.contains("f2"));
}

TEST_F(BackupTestFixture, RestoreBach)
{
    EXPECT_EQ(::system("mkdir dir"), 0);
    EXPECT_EQ(
        ::system("echo \"hello1\" > dir/f1 && echo \"hello2\" > dir/f2"), 0);
    std::vector<std::pair<std::string, std::string>> filelist;
    filelist.emplace_back("dir", "");
    filelist.emplace_back("dir/f1", "dir");
    filelist.emplace_back("dir/f2", "dir");
    BackupConfig config{"backup_test", ".", false, ""};

    backup->BackupBatch(config, filelist);
    Status s = backup->RestoreBatch("backup_test", {"dir"}, "./recover1");
    EXPECT_EQ(s.code, OK);
    EXPECT_EQ(::system("cmp dir/f1 recover1/dir/f1"), 0);
    EXPECT_EQ(::system("cmp dir/f2 recover1/dir/f2"), 0);
    EXPECT_EQ(::system("rm -rf dir recover1 backup_test"), 0);
}