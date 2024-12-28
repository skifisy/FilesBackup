#pragma once
#include <dirent.h>

#include <unordered_map>
#include <vector>

#include "backup.h"

namespace backup {
constexpr int BUFFER_SIZE = 1024;

class BackUpImpl : public BackUp
{
  public:
    BackUpImpl() = default;
    ~BackUpImpl() = default;
    // 将src中的所有文件保存到dest中
    void Copy(const std::string &src, const std::string &dest);
    // 将目录树中的文件保存到指定位置
    // TODO: 压缩、打包、加密
    void Save(const std::string &src, const std::string &dest);
    // 将目录树中的文件恢复到指定位置
    // TODO: 解压、解包、解密
    void Recover(const std::string &src, const std::string &dest);

  private:
    bool IsDirectory(const std::string &path);
    bool IsExist(const std::string &path);
    bool CheckSrcDestDir(const std::string &src, const std::string &dest);
    /**
     * @param ino_map inode号-文件路径
     * @param dir_list
     * copy过程中遇到的所有目录，pair.first为源目录路径，pair.second为目标目录路径
     */
    void CopyFiles(
        const std::string &src,
        const std::string &dest,
        std::unordered_map<ino_t, std::string> &ino_map,
        std::vector<std::pair<std::string, std::string>> &dir_list,
        const std::string &origin_src,
        const std::string &orgin_dest);
    void CopyFileContent(const std::string &src, const std::string &dest);
    static void
    CopyFileMetadata(const std::string &src, const std::string &dest);
    /**
     * 将src文件拷贝到dest目录下，包含文件元信息。
     * @param src 源文件
     * @param dest 目标目录
     */
    void CopyFileWithMetadata(const std::string &src, const std::string &dest);
    static void CopySymlinkWithMetadata(
        const std::string &src,
        const std::string &dest,
        const std::string &origin_src,
        const std::string &origin_dest);
    std::string ToFileName(const std::string &path);
    std::string
    AppendPath(const std::string &prefix, const std::string &filename);

    // void CopyOneFile(const )
    /* data */
};
} // namespace backup
