////
// @file schedule_task.h
// @brief
// 定时任务设计
//
// @author yuhaoze
// @email 743544510@qq.com
//
#pragma once
#include <string>
#include <ctime>
#include <vector>
#include <fstream>

namespace backup {
enum class TimeUnit
{
    SECOND,
    MINUTE,
    HOUR,
    DAY,
    WEEK,
    MONTH,
    YEAR
};

struct BackupTask
{
    std::string filename;
    std::time_t next_backup_time;
    TimeUnit unit;
    int interval; // 例如每两天: interval=2, unit=DAY
    bool isEncrypted;
    // 说明：对一个仅仅只有客户端的项目，进行定时的加密备份没有意义
    // 因为你没有办法既用对称加密，又在不知道密码的时候进行定时
    // 但是，如果扩展为网盘项目，则是有用的，因为存储在网盘上的pack
    // 是不知道密码的
    std::string password;
    std::string path; // 绝对路径

    size_t Dump(std::ofstream &ofs);
    size_t Load(std::ifstream &ifs);
};

// 全局唯一，启动gui时就获取tasklist
class TaskScheduler
{
  public:
    explicit TaskScheduler();
    ~TaskScheduler();
    size_t Dump(std::ofstream &ofs);
    size_t Load(std::ifstream &ifs);

  private:
    std::vector<BackupTask> tasklist;
    std::string task_path;
};
} // namespace backup
