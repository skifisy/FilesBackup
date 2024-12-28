

#pragma once
#include "file_meta.h"
#include "pack.h"
#include <vector>
#include <string>
namespace backup {

class FilePackImpl : public FilePack
{
  public:
    FilePackImpl() = default;
    ~FilePackImpl() = default;
    // 将某个文件/文件夹打包到某个位置上
    void Pack(std::string src, std::string dest);
    // 将某个打包文件解包到某个位置上
    void UnPack(std::string src, std::string dest);

    /**
     * @brief 批量打包文件
     * @param config 打包配置
     * @param src_path 需要打包的文件数组
     * @param target_path 打包文件的目的地址
     */
    void PackBatch(
        const PackConfig &config,
        const std::vector<std::string> &src_path,
        const std::string &target_path);
    void UnPackBatch();

  private:
    Path path_;
    BackupFileHeader file_header_;
};

} // namespace backup
