#include "config.h"
#include "file_sys.h"
#include <fstream>
namespace backup {

// 初始化静态成员变量
Config *Config::instance = nullptr;

Config *Config::getInstance()
{
    if (instance == nullptr) {
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        if (instance == nullptr) { instance = new Config(); }
    }
    return instance;
}

void Config::destroyInstance()
{
    delete instance;
    instance = nullptr;
}
Config::Config()
{
    ErrorCode ret = Access("config.json", Permission::READ);
    if (ret == NOT_EXIST) { throw Status{ret, "配置文件config.json不存在"}; }

    if (ret == NO_PERMISSION) {
        throw Status{ret, "无法访问配置文件config.json"};
    }
    std::ifstream ifs("config.json");
    json jsonData;
    ifs >> jsonData;
    configData.emplace("task_path", jsonData["task_path"]);
    configData.emplace("temp_path", jsonData["temp_path"]);
}
} // namespace backup
