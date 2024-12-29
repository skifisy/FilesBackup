#include "backup_impl.h"
#include "file_tree.h"
#include "compress.h"
#include "encrypt.h"
#include "cstring"
namespace backup {
size_t BackupHeader::Dump(std::ofstream &ofs)
{
    size_t ret = 0;
    ret += DumpString(magic, ofs);
    ret += DumpVar(is_encrypt, ofs);
    return ret;
}

size_t BackupHeader::Load(std::ifstream &ifs)
{
    size_t ret = 0;
    std::string mag;
    ret += LoadString(mag, ifs);
    memcpy(magic, mag.c_str(), mag.size());
    ret += LoadVar(is_encrypt, ifs);
    return ret;
}

// backup文件结构：
// Config
// backup file  // compressed or encrypted?
// - 先压缩，后加密，保证更高的压缩率

// packfile
// header+meta+data+footer
void BackUpImpl::BackupBatch(
    const BackupConfig &config,
    const std::vector<std::string> &src_path)
{
    std::string target_path =
        (Path(config.target_dir) / Path(config.backup_name)).ToString();
    // 1. 打包文件
    FileTree filetree;
    for (const std::string &path : src_path) {
        filetree.PackFileAdd(path, "", false);
    }
    std::string pack_path = target_path + "_pack_temp";
    std::ofstream packfile_ofs(pack_path, std::ios::binary);
    filetree.FullDump(packfile_ofs);
    packfile_ofs.flush();

    // 往backup file写入header信息
    std::ofstream target_file_ofs(target_path, std::ios::binary);
    BackupHeader header;
    header.is_encrypt = config.is_encrypt;
    header.Dump(target_file_ofs);

    // 2. 压缩文件
    std::ifstream packfile_ifs(pack_path, std::ios::binary);
    std::string compress_path(target_path + "_compress_temp");
    std::ofstream compress_file_ofs;
    if (config.is_encrypt) {
        compress_file_ofs.open(compress_path, std::ios::binary);
    } else {
        compress_file_ofs = std::move(target_file_ofs);
    }
    Compress compress(packfile_ifs, compress_file_ofs);
    compress.CompressFile();
    compress_file_ofs.flush();
    // 3. 加密文件
    if (config.is_encrypt) {
        std::ifstream compress_file_ifs(compress_path, std::ios::binary);
        Encrypt enc(compress_file_ifs, target_file_ofs);
        enc.AES_encrypt_file(config.password);
    }
    // 4. 删除临时文件
}

std::vector<std::shared_ptr<FileNode>>
BackUpImpl::GetFileList(const std::string &backup_path)
{
    FileTree tree;
    std::ifstream ifs(backup_path, std::ios::binary);
    return std::vector<std::shared_ptr<FileNode>>();
}

void BackUpImpl::RestoreBatch() {}
} // namespace backup
