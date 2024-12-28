////
// @file pack.h
// @brief
// 文件打包
//
// @author yuhaoze
// @email 743544510@qq.com
//
#pragma once
#include <string>
#include "file_meta.h"
namespace backup {
struct PackConfig
{
    std::string filename;
    std::string backPath;
    RegularTimeType timetype;
    bool isEncrypt;
    std::string password;
};

class FilePack
{
  public:
    virtual void Pack(std::string src, std::string dest) = 0;
    virtual void UnPack(std::string src, std::string dest) = 0;
    virtual void PackBatch(
        const PackConfig &config,
        const std::vector<std::string> &src_path,
        const std::string &target_path) = 0;
    virtual void UnPackBatch() = 0;
    FilePack() = default;
    ~FilePack() = default;
};
} // namespace backup
