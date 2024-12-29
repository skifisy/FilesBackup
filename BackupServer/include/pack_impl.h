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

  private:
    Path path_;
    BackupFileHeader file_header_;
};

} // namespace backup
