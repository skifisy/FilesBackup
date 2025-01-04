#include <gtest/gtest.h>
#include <ctime>
#define private public
#include "schedule_task.h"
#include "backup_impl.h"
#include <thread>
using namespace backup;
using namespace std::chrono;
void CheckTask(const BackupTask &t1, const BackupTask &t2)
{
    EXPECT_EQ(t1.backup_path, t2.backup_path);
    EXPECT_EQ(
        duration_cast<seconds>(t1.next_backup_time.time_since_epoch()),
        duration_cast<seconds>(t2.next_backup_time.time_since_epoch()));
    EXPECT_EQ(t1.unit, t2.unit);
    EXPECT_EQ(t1.interval, t2.interval);
    EXPECT_EQ(t1.isEncrypted, t2.isEncrypted);
    EXPECT_EQ(t1.password, t2.password);
    EXPECT_EQ(t1.run, t2.run);
    EXPECT_EQ(t1.status, t2.status);
}

TEST(TaskSchedulerTest, TaskDumpAndLoad)
{
    EXPECT_EQ(::system("rm -f task_list.db"), 0);
    TaskScheduler scheduler;
    BackupTask *t1 = new BackupTask("n1", 1, TimeUnit::DAY, false, "");
    BackupTask *t2 = new BackupTask{"n2", 2, TimeUnit::SECOND, true, "hello"};
    BackupTask *t3 = new BackupTask{"n3", 3, TimeUnit::DAY, false, ""};
    BackupTask *t4 = new BackupTask{"n4", 4, TimeUnit::DAY, false, ""};
    scheduler.AddTask(t1);
    scheduler.AddTask(t2);
    scheduler.AddTask(t3);
    scheduler.AddTask(t4);
    std::ofstream ofs(scheduler.task_path, std::ios::binary);
    scheduler.Dump(ofs);
    ofs.close();
    TaskScheduler scheduler_recover;
    for (size_t i = 0; i < scheduler.task_list.size(); i++) {
        CheckTask(*scheduler.task_list[i], *scheduler_recover.task_list[i]);
    }
}

TEST(TaskSchedulerTest, TaskSchedule)
{
    EXPECT_EQ(::system("rm -f task_list.db"), 0);
    BackUp *back = new BackUpImpl();
    EXPECT_EQ(::system("mkdir dir"), 0);
    EXPECT_EQ(
        ::system("echo \"hello1\" > dir/f1 && echo \"hello2\" > dir/f2"), 0);
    std::vector<BackupData> filelist;
    filelist.emplace_back("dir", "");
    filelist.emplace_back("dir/f1", "dir");
    filelist.emplace_back("dir/f2", "dir");
    BackupConfig config{"backup_test", ".", false, ""};
    back->BackupBatch(config, filelist);
    TaskScheduler scheduler;
    BackupTask *task =
        new BackupTask("backup_test", 200, TimeUnit::MILISECOND, false, "");
    scheduler.AddTask(task);
    scheduler.StartThread();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(Access("backup_test_0", EXIST), OK);
    EXPECT_EQ(Access("backup_test_1", EXIST), OK);
    EXPECT_EQ(::system("rm -rf backup_test* dir"), 0);
}
