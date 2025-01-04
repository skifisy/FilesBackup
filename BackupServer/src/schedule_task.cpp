#include "schedule_task.h"
#include <iostream>
#include <fstream>
#include <sys/timerfd.h>
#include "file_meta.h"
#include "config.h"
#include "backup_impl.h"
using namespace std::chrono;
namespace backup {
TaskScheduler::TaskScheduler()
    : timer_fd(-1)
{
    // 获取配置
    Config *config = Config::getInstance();
    task_path = config->getConfigData("task_path");
    std::ifstream taskfile(task_path, std::ios::binary);
    if (taskfile.is_open()) Load(taskfile);
}

TaskScheduler::~TaskScheduler()
{
    std::ofstream ofs(task_path, std::ios::binary);
    if (ofs.is_open())
        Dump(ofs);
    else
        std::cerr << "can't open file " << task_path << std::endl;

    if (timer_fd != -1) {
        ShutDown();
        if (worker_thread.joinable()) worker_thread.join();
        close(timer_fd);
    }
    for (auto &task : task_list) {
        if (task) delete task;
    }
}

void TaskScheduler::AddTask(BackupTask *task)
{
    task_list.emplace_back(task);
    std::unique_lock<std::mutex> lock(task_queue_mutex);
    task_queue.emplace(task);
    auto timepoint = UserTimeToTimepoint(task->interval, task->unit);
    task->next_backup_time = timepoint;
    auto &top_task = task_queue.top();
    if (timer_fd != -1 && top_task == task) { UpdateTimer(timepoint); }
}

bool TaskScheduler::StartThread()
{
    timer_fd = timerfd_create(CLOCK_REALTIME, 0);
    if (timer_fd == -1) {
        std::cerr << "Timer can not be initialized!" << std::endl;
        return false;
    }
    std::unique_lock<std::mutex> lock(task_queue_mutex);
    if (!task_queue.empty()) {
        UpdateTimer(task_queue.top()->next_backup_time);
    }
    lock.unlock();
    worker_thread = std::thread(&TaskScheduler::SchedulerThread, this);
    return true;
}

std::vector<BackupTask> TaskScheduler::GetTaskList()
{
    std::vector<BackupTask> ret;
    std::unique_lock<std::mutex> lock(task_queue_mutex);
    for (BackupTask *task : task_list) {
        ret.emplace_back(*task);
    }
    return ret;
}

void TaskScheduler::DeleteTask(const std::string &backupPath)
{
    std::unique_lock lock(task_queue_mutex);
    for (auto it = task_list.begin(); it != task_list.end(); it++) {
        BackupTask *task = *it;
        if (task->backup_path == backupPath) {
            task->run = false;
            task->status = "已取消";
            task_list.erase(it);
            break;
        }
    }
}

void TaskScheduler::DeleteAll()
{
    std::unique_lock lock(task_queue_mutex);
    task_queue =
        std::priority_queue<BackupTask *, std::vector<BackupTask *>, Compare>();
    lock.unlock();
    for (auto task : task_list) {
        delete task;
    }
    task_list.clear();
}

void TaskScheduler::ShutDown()
{
    std::unique_lock<std::mutex> lock;
    shutdown = true;
    UpdateTimer(system_clock::now());
}

size_t TaskScheduler::Dump(std::ofstream &ofs)
{
    size_t ret = 0;
    size_t count = task_list.size();
    ret += DumpVar(count, ofs);
    for (size_t i = 0; i < count; i++) {
        ret += task_list[i]->Dump(ofs);
    }
    return ret;
}
size_t TaskScheduler::Load(std::ifstream &ifs)
{
    size_t ret = 0;
    size_t count;
    ret += LoadVar(count, ifs);
    for (size_t i = 0; i < count; i++) {
        BackupTask *task = new BackupTask;
        ret += task->Load(ifs);
        if (task->run)
            AddTask(task);
        else
            delete task;
    }
    return ret;
}
void TaskScheduler::UpdateTimer(
    const std::chrono::time_point<std::chrono::system_clock> &tm)
{
    auto now = std::chrono::system_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(tm - now);
    if (duration.count() <= 0) {
        duration = std::chrono::milliseconds(
            1); // 如果任务已经到期，保证定时器最小为 1 毫秒
    }
    struct itimerspec new_value;
    new_value.it_value.tv_sec = duration.count() / 1000;
    new_value.it_value.tv_nsec =
        (duration.count() % 1000) * 1000000; // 转换为纳秒
    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 0;

    if (timerfd_settime(timer_fd, 0, &new_value, nullptr) == -1) {
        perror("timerfd_settime");
        exit(EXIT_FAILURE);
    }
}
void TaskScheduler::SchedulerThread()
{
    uint64_t expirations;

    while (true) {
        ssize_t s = read(timer_fd, &expirations, sizeof(expirations));
        if (s != sizeof(expirations)) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        std::unique_lock<std::mutex> lock(task_queue_mutex);
        if (shutdown) break;
        // 取出所有到期的任务并执行
        auto now = std::chrono::system_clock::now();
        std::vector<BackupTask *> tasks;
        while (!task_queue.empty() &&
               task_queue.top()->next_backup_time <= now) {
            BackupTask *task = task_queue.top();
            task_queue.pop();
            if (task->run)
                tasks.emplace_back(task);
            else
                delete task;
        }
        lock.unlock();
        for (auto &task : tasks) {
            DoBackupTask(*task);
            // 执行完毕，重新入队
            task->next_backup_time =
                UserTimeToTimepoint(task->interval, task->unit);
        }
        lock.lock();
        // 重新入队
        for (auto &task : tasks) {
            if (task->run)
                task_queue.emplace(task);
            else
                delete task;
        }
        // 如果队列中还有任务，更新定时器的触发时间
        if (!task_queue.empty()) {
            UpdateTimer(task_queue.top()->next_backup_time);
        }
    }
}
void TaskScheduler::DoBackupTask(const BackupTask &task)
{
    BackUpImpl back;
    back.ReBackupFile(task.backup_path, task.isEncrypted, task.password);
}
size_t BackupTask::Dump(std::ofstream &ofs)
{
    size_t ret = 0;
    ret += DumpString(backup_path, ofs);
    struct timeval tv = chrono_to_timeval(next_backup_time);
    ret += DumpVar(tv, ofs);
    ret += DumpVar(static_cast<char>(unit), ofs);
    ret += DumpVar(interval, ofs);
    ret += DumpVar(isEncrypted, ofs);
    ret += DumpString(password, ofs);
    ret += DumpVar(run, ofs);
    ret += DumpString(status, ofs);
    return ret;
}
size_t BackupTask::Load(std::ifstream &ifs)
{
    size_t ret = 0;
    ret += LoadString(backup_path, ifs);
    struct timeval tv;
    ret += LoadVar(tv, ifs);
    next_backup_time = timeval_to_chrono(tv);
    char time_unit;
    ret += LoadVar(time_unit, ifs);
    unit = static_cast<TimeUnit>(time_unit);
    ret += LoadVar(interval, ifs);
    ret += LoadVar(isEncrypted, ifs);
    ret += LoadString(password, ifs);
    ret += LoadVar(run, ifs);
    ret += LoadString(status, ifs);
    return ret; 
}
std::chrono::system_clock::time_point
UserTimeToTimepoint(int interval, TimeUnit unit)
{
    milliseconds dur;
    switch (unit) {
    case TimeUnit::MILISECOND:
        dur = milliseconds(interval);
        break;
    case TimeUnit::SECOND:
        dur = seconds(interval);
        break;
    case TimeUnit::MINUTE:
        dur = minutes(interval);
        break;
    case TimeUnit::HOUR:
        dur = hours(interval);
        break;
    case TimeUnit::DAY:
        dur = days(interval);
        break;
    case TimeUnit::WEEK:
        dur = weeks(interval);
        break;
    case TimeUnit::MONTH:
        dur = months(interval);
        break;
    case TimeUnit::YEAR:
        dur = years(interval);
        break;
    default:
        break;
    }
    return system_clock::now() + dur;
}

struct timeval
chrono_to_timeval(const std::chrono::system_clock::time_point &time_point)
{
    // 获取自纪元以来的时间段
    auto duration = time_point.time_since_epoch();

    // 转换为秒数
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);

    // 转换为毫秒数
    auto millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration);

    struct timeval tv;
    tv.tv_sec = seconds.count();                                   // 秒部分
    tv.tv_usec = (millis.count() - seconds.count() * 1000) * 1000; // 微秒部分

    return tv;
}
std::chrono::system_clock::time_point
timeval_to_chrono(const struct timeval &tv)
{
    // 将 timeval 转换为 chrono::seconds 和 chrono::microseconds
    std::chrono::seconds sec(tv.tv_sec);
    std::chrono::microseconds usec(tv.tv_usec);

    // 合并秒和微秒
    return std::chrono::system_clock::time_point(sec + usec);
}

std::string GetTimeUnitTag(TimeUnit unit)
{
    switch (unit) {
    case TimeUnit::NONE:
        return "未定时";
    case TimeUnit::SECOND:
        return "秒";
    case TimeUnit::MINUTE:
        return "分钟";
    case TimeUnit::HOUR:
        return "小时";
    case TimeUnit::DAY:
        return "天";
    case TimeUnit::WEEK:
        return "周";
    case TimeUnit::MONTH:
        return "月";
    case TimeUnit::YEAR:
        return "年";
    case TimeUnit::MILISECOND:
        return "微秒";
    default:
        return "错误";
    }
}

} // namespace backup
