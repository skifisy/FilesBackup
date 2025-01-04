////
// @file schedule_task.h
// @brief
// 定时任务设计
//
// @author hz
// @email skyfishine@163.com
//
#pragma once
#include <string>
#include <ctime>
#include <vector>
#include <fstream>
#include <queue>
#include <thread>
#include <sys/time.h>
namespace backup {
enum class TimeUnit
{
    NONE, // 不定时
    SECOND,
    MINUTE,
    HOUR,
    DAY,
    WEEK,
    MONTH,
    YEAR,
    MILISECOND, // 毫秒
};

struct BackupTask
{
    std::string backup_path;
    int interval; // 例如每两天: interval=2, unit=DAY
    TimeUnit unit;
    bool isEncrypted;
    // 说明：对一个仅仅只有客户端的项目，进行定时的加密备份没有意义
    // 因为你没有办法既用对称加密，又在不知道密码的时候进行定时
    // 但是，如果扩展为网盘项目，则是有用的，因为存储在网盘上的pack
    // 是不知道密码的
    std::string password;
    std::chrono::system_clock::time_point next_backup_time;
    std::string status = "正常"; // 当前的任务状态（对于可能出错的情况）
    bool run = true;             // 出错的task就不执行了
    BackupTask() = default;
    BackupTask(
        const std::string &backup_path,
        int interval,
        TimeUnit unit,
        bool isEncrypt,
        const std::string &password)
        : backup_path(backup_path)
        , interval(interval)
        , unit(unit)
        , isEncrypted(isEncrypt)
        , password(password)
    {}
    // 用于任务排序，确保队列中任务按执行时间排序
    bool operator>(const BackupTask &other) const
    {
        return next_backup_time > other.next_backup_time;
    }
    size_t Dump(std::ofstream &ofs);
    size_t Load(std::ifstream &ifs);
};

// 自定义比较器：小根堆（最小堆）
struct Compare
{
    bool operator()(const BackupTask *lhs, const BackupTask *rhs)
    {
        return *lhs > *rhs;
    }
};

// 全局唯一，启动gui时就获取tasklist
class TaskScheduler
{
  public:
    explicit TaskScheduler();
    ~TaskScheduler();
    void AddTask(BackupTask *task);
    bool StartThread();
    // 获取tasklist的拷贝（防止竞争）
    std::vector<BackupTask> GetTaskList();
    void DeleteTask(const std::string &backupPath);
    void DeleteAll();

  private:
    size_t Dump(std::ofstream &ofs);
    size_t Load(std::ifstream &ifs);
    void
    UpdateTimer(const std::chrono::time_point<std::chrono::system_clock> &tm);
    // 线程循环
    void SchedulerThread();
    // 单次定时任务执行
    void DoBackupTask(const BackupTask &task);
    void ShutDown();
    std::string task_path;
    std::vector<BackupTask *> task_list;

    int timer_fd;              // 定时器文件描述符
    std::thread worker_thread; // 工作线程
    std::priority_queue<BackupTask *, std::vector<BackupTask *>, Compare>
        task_queue;              // 任务队列
    std::mutex task_queue_mutex; // 保护任务队列的互斥锁
    bool shutdown = false;
};

std::chrono::time_point<std::chrono::system_clock>
UserTimeToTimepoint(int interval, TimeUnit unit);
struct timeval
chrono_to_timeval(const std::chrono::system_clock::time_point &time_point);
std::chrono::system_clock::time_point
timeval_to_chrono(const struct timeval &tv);

std::string GetTimeUnitTag(TimeUnit unit);
} // namespace backup
