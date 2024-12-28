

#pragma once
#include "file_meta.h"
#include "pack.h"
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
    void PackBatch();
    void UnPackBatch();

  private:
    Path path_;
    BackupFileHeader file_header_;
};

} // namespace backup
