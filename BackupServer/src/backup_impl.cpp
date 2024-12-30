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
Status BackUpImpl::BackupBatch(
    const BackupConfig &config,
    const std::vector<std::string> &src_path)
{
    std::string target_path =
        (Path(config.target_dir) / Path(config.backup_name)).ToString();
    // 1. 打包文件
    FileTree filetree;
    // TODO: 这里有问题，gui传过来的时候应该传入src和dest
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
    RemoveFile(std::move(pack_path));
    RemoveFile(std::move(compress_path));

    return {OK, ""};
}

Status BackUpImpl::BackupBatch(
    const BackupConfig &config,
    const std::vector<std::pair<std::string, std::string>> &src_path) {
       return {OK, ""}; 
    }

std::tuple<Status, std::shared_ptr<FileNode>> BackUpImpl::GetFileList(
    const std::string &backup_path,
    const std::string &password)
{
    std::string pack_path;
    FileTree tree;
    try {
        pack_path = RecoverToPackFile(backup_path, password);
        std::ifstream ifs(pack_path);
        tree.Load(ifs);
        RemoveFile(pack_path);
    } catch (const Status &status) {
        if (!pack_path.empty()) { RemoveFile(pack_path); }
        return {status, nullptr};
    }
    return std::make_tuple(Status{OK, ""}, tree.GetRootNode());
}

std::tuple<Status, bool> BackUpImpl::isEncrypted(const std::string &backup_path)
{
    std::ifstream ifs(backup_path, std::ios::binary);
    if (!ifs.is_open()) {
        return {{NOT_EXIST, "文件" + backup_path + "不存在"}, false};
    }
    BackupHeader header;
    header.Load(ifs);
    if (strncmp(header.magic, "BUK", 3) != 0) {
        return {{FORMAT_ERROR, backup_path + "不是打包文件"}, false};
    }

    return {{OK, ""}, header.is_encrypt};
}

void BackUpImpl::RestoreBatch() {}
std::string BackUpImpl::RecoverToPackFile(
    const std::string &backup_path,
    const std::string &password)
{
    std::ifstream ifs(backup_path, std::ios::binary);
    if (!ifs.is_open()) {
        throw Status{NOT_EXIST, "文件" + backup_path + "不存在"};
    }
    BackupHeader header;
    header.Load(ifs);
    if (strncmp("BUK", header.magic, 3) != 0) {
        throw Status{
            FORMAT_ERROR,
            "文件" + backup_path + "不是备份文件，请选择正确的文件"};
    }
    if (password.empty() && header.is_encrypt) {
        throw Status{ENCRYPTED, "密码不能为空"};
    }

    if (password.empty()) {
        std::string uncompress_path = backup_path + "_uncompress_temp";
        std::ofstream ofs(uncompress_path, std::ios::binary);
        Compress comp(ifs, ofs);
        comp.DecompressFile();
        return uncompress_path;
    } else {
        std::string decrypted_path = backup_path + "_decrypted_temp";
        std::ofstream ofs(decrypted_path, std::ios::binary);
        Encrypt enc(ifs, ofs);
        ofs.close();
        bool dec_ret = enc.AES_decrypt_file(password);
        if (!dec_ret) {
            RemoveFile(decrypted_path);
            throw Status{PASSWORD_ERROR, "密码错误"};
        }
        std::string uncompress_path = backup_path + "_uncompress_temp";
        std::ifstream decrypted_file_ifs(decrypted_path, std::ios::binary);
        std::ofstream uncompress_file_ofs(uncompress_path, std::ios::binary);
        Compress comp(decrypted_file_ifs, uncompress_file_ofs);
        comp.DecompressFile();
        return uncompress_path;
    }
}
} // namespace backup
