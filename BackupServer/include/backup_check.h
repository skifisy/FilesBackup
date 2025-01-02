////
// @file file_check.h
// @brief
// 文件校验
//
// @author hz
// @email skyfishine@163.com

#pragma once
#include <string>
#include <vector>
#include "file_meta.h"
#include "file_tree.h"
namespace backup {
enum class CheckType
{
    ADD,         // 新增
    DELETE,      // 删除
    META_UPDATE, // 文件元信息修改
    DATA_UPDATE, // 文件内容修改
    UPDATE,      // 同时发生元信息&内容修改
    OTHER,       // 其他
    OK,          // 无修改
};

std::string GetCheckTypeTag(CheckType type);
struct CheckResult
{
    CheckType type;
    std::string origin_path;
    std::string backup_path;
    std::string detail;
};

class BackupCheck
{
  public:
    BackupCheck() = default;
    ~BackupCheck() = default;
    std::pair<Status, std::vector<CheckResult>> CheckBackupFile(
        const std::string &backup_path,
        const std::string &password = "");

  private:
    void CheckBackupFile(
        std::vector<CheckResult> &result,
        std::shared_ptr<FileNode> root);

    CheckResult MakeResult(
        CheckType type,
        FileType filetype,
        const std::string &origin_path,
        const std::string &backup_path,
        const std::string &details = "");

    void CheckFileMeta(CheckResult &result, const FileMetadata &meta);
};

} // namespace backup
