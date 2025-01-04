#include <gtest/gtest.h>

#include "backup_check.h"
#include "backup_impl.h"
using namespace backup;

TEST(BackupCheck, CheckBackupFile)
{
    EXPECT_EQ(::system("rm -rf tt dd f1"), 0);
    EXPECT_EQ(::system("echo \"hello\" > f1 && mkdir tt && mkfifo tt/fo"), 0);

    BackUp *back = new BackUpImpl();
    BackupConfig config = {"pack", ".", false, ""};
    std::vector<BackupData> paths = {{"f1", "/"}, {"tt", "/"}, {"tt/fo", "tt"}};
    back->BackupBatch(config, paths);
    back->RestoreBatch("pack", {{"/f1"}, {"/tt/fo"}}, "dd");
    EXPECT_EQ(::system("cmp f1 dd/f1"), 0);
    BackupCheck check;
    auto [s1, result1] = check.CheckBackupFile("pack");
    EXPECT_EQ(s1.code, OK);
    EXPECT_TRUE(result1.empty());
    EXPECT_EQ(::system("rm -rf tt"), 0);
    auto [s2, result2] = check.CheckBackupFile("pack");
    EXPECT_EQ(s2.code, OK);
    EXPECT_EQ(result2.size(), 1);

    EXPECT_EQ(::system("rm -f f1"), 0);
    auto [s3, result3] = check.CheckBackupFile("pack");
    EXPECT_EQ(s3.code, OK);
    EXPECT_EQ(result3.size(), 2);
    // for (auto &r : result) {
    //     std::cout << GetCheckTypeTag(r.type) << "; " << r.origin_path << "; "
    //               << r.backup_path << "; " << r.detail << std::endl;
    // }
    EXPECT_EQ(::system("rm -rf dd pack"), 0);
}