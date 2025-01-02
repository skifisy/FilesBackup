////
// @file config.h
// @brief
// 配置信息
//
// @author hz
// @email skyfishine@163.com
//
#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include "error_code.h"
using json = nlohmann::json;
namespace backup {
class Config
{
  public:
    // 获取单例实例的公共静态方法
    static Config *getInstance();

    std::string getConfigData(const std::string &key)
    {
        if (configData.contains(key))
            return configData[key];
        else
            throw Status{ERROR, "无法找到配置项"};
    }

    // 手动销毁单例实例
    static void destroyInstance();

  private:
    // 私有的静态实例指针
    static Config *instance;

    std::unordered_map<std::string, std::string> configData;

    // 私有构造函数，防止外部实例化
    Config();

    // 禁用拷贝构造函数和赋值运算符
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;
};

} // namespace backup
