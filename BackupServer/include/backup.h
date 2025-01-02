////
// @file backup.h
// @brief
// 文件备份（主要的api）
//
// @author hz
// @email skyfishine@163.com
//

#pragma once
#include <string>
#include "file_meta.h"
#include "error_code.h"
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
    virtual Status BackupBatch(
        const BackupConfig &config,
        const std::vector<std::string> &src_path) = 0;

    virtual Status BackupBatch(
        const BackupConfig &config,
        const std::vector<std::pair<std::string, std::string>> &src_path) = 0;

    /**
     * @brief 将打包文件中的部分文件批量恢复到某个文件夹中
     * @param backup_path 备份文件的路径
     * @param pack_paths 需要恢复的文件路径（相对于打包文件来说）
     * @param target_dir 恢复到的目标目录
     * @param password 如果文件被加密，需要填写
     */
    virtual Status RestoreBatch(
        const std::string &backup_path,
        const std::vector<std::string> &pack_paths,
        const std::string &target_dir,
        const std::string &password = "") = 0;

    virtual std::tuple<Status, std::shared_ptr<FileNode>> GetFileList(
        const std::string &backup_path,
        const std::string &password = "") = 0;

    virtual std::tuple<Status, bool>
    isEncrypted(const std::string &backup_path) = 0;

    BackUp() = default;
    virtual ~BackUp() = default;
};

} // namespace backup
