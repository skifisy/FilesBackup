#include "schedule_task.h"
#include <iostream>
#include <fstream>
#include "file_meta.h"
#include "config.h"
namespace backup {
TaskScheduler::TaskScheduler()
{
    // 获取配置
    Config* config = Config::getInstance();
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
}

size_t TaskScheduler::Dump(std::ofstream &ofs)
{
    size_t ret = 0;
    size_t count = tasklist.size();
    ret += DumpVar(count, ofs);
    for (size_t i = 0; i < count; i++) {
        ret += tasklist[i].Dump(ofs);
    }
    return ret;
}
size_t TaskScheduler::Load(std::ifstream &ifs)
{
    size_t ret = 0;
    size_t count;
    ret += LoadVar(count, ifs);
    tasklist.resize(count);
    for (size_t i = 0; i < count; i++) {
        ret += tasklist[i].Load(ifs);
    }
    return ret;
}
size_t BackupTask::Dump(std::ofstream &ofs)
{
    size_t ret = 0;
    ret += DumpString(filename, ofs);
    ret += DumpVar(next_backup_time, ofs);
    ret += DumpVar(static_cast<char>(unit), ofs);
    ret += DumpVar(interval, ofs);
    ret += DumpVar(isEncrypted, ofs);
    // 加密文件的定时备份？
    ret += DumpString(password, ofs);
    ret += DumpString(path, ofs);
    return ret;
}
size_t BackupTask::Load(std::ifstream &ifs)
{
    size_t ret = 0;
    ret += LoadString(filename, ifs);
    ret += LoadVar(next_backup_time, ifs);
    char time_unit;
    ret += LoadVar(time_unit, ifs);
    unit = static_cast<TimeUnit>(time_unit);
    ret += LoadVar(interval, ifs);
    ret += LoadVar(isEncrypted, ifs);
    ret += LoadString(password, ifs);
    ret += LoadString(path, ifs);
    return ret;
}
} // namespace backup
