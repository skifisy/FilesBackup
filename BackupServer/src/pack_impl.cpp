#include <cstring>
#include <iostream>

#include "file_sys.h"
#include "pack.h"
#include "pack_impl.h"
namespace backup {

void FilePackImpl::Pack(std::string src, std::string dest)
{
    // 1. 遍历src，构建文件树
    // 2. 将文件树打包为一个大文件

    // 思路2：边遍历，边打包（问题：各种data的offset如何计算？）
}

void FilePackImpl::UnPack(std::string src, std::string dest) {}

void FilePackImpl::PackBatch(
    const PackConfig &config,
    const std::vector<std::string> &src_path,
    const std::string &target_path)
{
    
}

} // namespace backup
