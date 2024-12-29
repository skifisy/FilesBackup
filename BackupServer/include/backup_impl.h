#pragma once
#include <dirent.h>

#include <unordered_map>
#include <vector>

#include "backup.h"

namespace backup {

class BackUpImpl : public BackUp
{
  public:
    /**
     * @brief 批量打包文件
     * @param config 打包配置
     * @param src_path 需要打包的文件数组
     * @param target_path 打包文件的目的地址
     */
    void BackupBatch(
        const BackupConfig &config,
        const std::vector<std::string> &src_path);

    /**
     * @brief 根据打包文件，获取文件list
     * @param backup_path 打包文件的路径
     */
    std::vector<std::shared_ptr<FileNode>>
    GetFileList(const std::string &backup_path);

    /**
     * @brief
     */
    void RestoreBatch();

    BackUpImpl() = default;
    ~BackUpImpl() = default;
};
} // namespace backup
