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
     */
    Status BackupBatch(
        const BackupConfig &config,
        const std::vector<std::string> &src_path);

    /**
     * @brief 批量打包文件
     * @param config 打包配置
     */
    Status BackupBatch(
        const BackupConfig &config,
        const std::vector<BackupData> &src_path);

    /**
     * @brief 根据打包文件，获取文件list
     * @param backup_path 打包文件的路径
     */
    std::tuple<Status, std::shared_ptr<FileNode>> GetFileList(
        const std::string &backup_path,
        const std::string &password = ""); // 这里可以不用带有默认参数？

    std::tuple<Status, bool> isEncrypted(const std::string &backup_path);

    /**
     * @brief 将打包文件中的部分文件批量恢复到某个文件夹中
     * @param backup_path 备份文件的路径
     * @param pack_paths 需要恢复的文件路径（相对于打包文件来说）
     * @param target_dir 恢复到的目标目录
     * @param password 如果文件被加密，需要填写
     */
    Status RestoreBatch(
        const std::string &backup_path,
        const std::vector<std::string> &pack_paths,
        const std::string &target_dir,
        const std::string &password = "");

    /**
     * @brief 重新备份文件（定时备份）
     */
    Status ReBackupFile(
        const std::string &backup_path,
        bool isEncrypt,
        const std::string &password);

    BackUpImpl();
    ~BackUpImpl() = default;

  private:
    void BackupTreeDetail(
        const std::string &target_path,
        backup::FileTree &filetree,
        bool is_encrypt,
        const std::string &password);
    // 备份文件->解密->解压缩
    /**
     * @return 解密&解压后的文件路径
     */
    std::string RecoverToPackFile(
        const std::string &backup_path,
        const std::string &password = "");

    void CheckFilePermission(const std::string &path, int permissions);

    void ReBackupFile(
        std::shared_ptr<FileNode> node,
        FileTree &tree,
        std::string pack_dir);
    std::string temp_path;
};
} // namespace backup
