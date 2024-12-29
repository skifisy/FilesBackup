#include <gtest/gtest.h>
#include <ctime>
#define private public
#include "schedule_task.h"
using namespace backup;

void CheckTask(const BackupTask &t1, const BackupTask &t2)
{
    EXPECT_EQ(t1.filename, t2.filename);
    EXPECT_EQ(t1.next_backup_time, t2.next_backup_time);
    EXPECT_EQ(t1.unit, t2.unit);
    EXPECT_EQ(t1.interval, t2.interval);
    EXPECT_EQ(t1.isEncrypted, t2.isEncrypted);
    EXPECT_EQ(t1.password, t2.password);
    EXPECT_EQ(t1.path, t2.path);
}

TEST(TaskScheduler, TaskDumpAndLoad)
{
    TaskScheduler scheduler;
    BackupTask t1 = {"n1", time(NULL), TimeUnit::DAY, 2, false, "", "/ff/n1"};
    BackupTask t2 = {
        "n2", time(NULL), TimeUnit::SECOND, 2, true, "hello", "/ff/n2"};
    BackupTask t3 = {"n3", time(NULL), TimeUnit::DAY, 2, false, "", "/ff/n1"};
    BackupTask t4 = {"n4", time(NULL), TimeUnit::DAY, 2, false, "", "/ff/n1"};
    scheduler.tasklist.emplace_back(t1);
    scheduler.tasklist.emplace_back(t2);
    scheduler.tasklist.emplace_back(t3);
    scheduler.tasklist.emplace_back(t4);
    std::ofstream ofs(scheduler.task_path, std::ios::binary);
    scheduler.Dump(ofs);
    ofs.close();
    TaskScheduler scheduler_recover;
    for (size_t i = 0; i < scheduler.tasklist.size(); i++) {
        CheckTask(scheduler.tasklist[i], scheduler_recover.tasklist[i]);
    }
    
}