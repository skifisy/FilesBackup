#include <gtest/gtest.h>
#include "backup_impl.h"
#include "encrypt.h"
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
    EXPECT_EQ(::system("rm -rf dir"), 0);
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
    std::vector<BackupData> filelist;
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
    std::vector<BackupData> filelist;
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
    std::vector<BackupData> filelist;
    filelist.emplace_back("dir", "");
    filelist.emplace_back("dir/f1", "dir");
    filelist.emplace_back("dir/f2", "dir");
    BackupConfig config{"backup_test", ".", false, ""};

    backup->BackupBatch(config, filelist);
    Status s = backup->RestoreBatch("backup_test", {"dir"}, "./recover1");
    EXPECT_EQ(s.code, OK);
    EXPECT_EQ(::system("cmp dir/f1 recover1/dir/f1"), 0);
    EXPECT_EQ(::system("cmp dir/f2 recover1/dir/f2"), 0);
    // 验证哈希值
    std::ifstream ifs1("dir/f1", std::ios::binary),
        ifs2("recover1/dir/f1", std::ios::binary);
    unsigned char hash1[SHA256_SIZE], hash2[SHA256_SIZE];
    compute_file_sha256(ifs1, hash1);
    compute_file_sha256(ifs2, hash2);
    EXPECT_EQ(::memcmp(hash1, hash2, SHA256_SIZE), 0);
    ifs1.open("dir/f2", std::ios::binary);
    ifs2.open("recover1/dir/f2", std::ios::binary);
    compute_file_sha256(ifs1, hash1);
    compute_file_sha256(ifs2, hash2);
    EXPECT_EQ(::memcmp(hash1, hash2, SHA256_SIZE), 0);
    EXPECT_EQ(::system("rm -rf dir recover1 backup_test"), 0);
}

TEST(BackupImplTest, ReBackupTest)
{
    EXPECT_EQ(::system("rm -rf re1 re2 dir backup*"), 0);
    BackUpImpl back;
    EXPECT_EQ(::system("mkdir dir"), 0);
    EXPECT_EQ(
        ::system("echo \"hello1\" > dir/f1 && echo \"hello2\" > dir/f2 && "
                 "mkfifo dir/fo"),
        0);
    std::vector<BackupData> filelist;
    filelist.emplace_back("dir", "");
    filelist.emplace_back("dir/f1", "dir");
    filelist.emplace_back("dir/f2", "dir");
    filelist.emplace_back("dir/fo", "dir");
    BackupConfig config{"backup_test", ".", false, ""};
    back.BackupBatch(config, filelist);
    EXPECT_EQ(::system("echo \"hello3\" > dir/f3 && rm -f dir/f2"), 0);
    back.ReBackupFile("backup_test", false, "");
    Status s = back.RestoreBatch("backup_test", {"dir"}, "./re1");
    EXPECT_EQ(s.code, OK);
    s = back.RestoreBatch("backup_test_0", {"dir"}, "./re2");
    EXPECT_EQ(s.code, OK);

    EXPECT_EQ(Access("re1/dir/f2", READ), NOT_EXIST);
    EXPECT_EQ(Access("re1/dir/f3", READ), OK);
    EXPECT_EQ(Access("re1/dir/fo", READ), OK);
    EXPECT_EQ(Access("re2/dir/f1", READ), OK);
    EXPECT_EQ(Access("re2/dir/f3", READ), NOT_EXIST);
    EXPECT_EQ(Access("re2/dir/fo", READ), OK);
    back.ReBackupFile("backup_test", false, "");
    EXPECT_EQ(Access("backup_test_1", EXIST), OK);
    EXPECT_EQ(::system("rm -rf re1 re2 dir backup*"), 0);
}