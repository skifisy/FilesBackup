////
// @file backup.h
// @brief
// 文件备份（主要的api）
//
// @author yuhaoze
// @email 743544510@qq.com
//

#pragma once
#include <string>
#include "file_meta.h"
#include "file_tree.h"
namespace backup {

struct BackupConfig
{
    // 打包文件的名字
    std::string backup_name;
    // 打包文件的存储地址
    std::string target_dir;
    // 是否加密
    bool is_encrypt;
    // 文件密码
    std::string password;
};

struct BackupHeader
{
    char magic[4] = "BUK";
    bool is_encrypt;
    // hash
    size_t Dump(std::ofstream &ofs);
    size_t Load(std::ifstream &ifs);
};

class BackUp
{
  public:
    virtual void BackupBatch(
        const BackupConfig &config,
        const std::vector<std::string> &src_path) = 0;

    virtual void RestoreBatch() = 0;
    virtual std::vector<std::shared_ptr<FileNode>>
    GetFileList(const std::string &backup_path) = 0;

    BackUp() = default;
    virtual ~BackUp() = default;
};

} // namespace backup
