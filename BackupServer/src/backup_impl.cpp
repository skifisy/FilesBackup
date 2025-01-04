#include "backup_impl.h"
#include "compress.h"
#include "config.h"
#include "cstring"
#include "encrypt.h"
#include "file_tree.h"
namespace backup {
class FilesCleaner
{
  public:
    FilesCleaner() = default;
    ~FilesCleaner()
    {
        for (const std::string &f : files) {
            RemoveFile(f);
        }
    }
    void AddFile(const std::string &path) { files.emplace_back(path); }

  private:
    std::vector<std::string> files;
};
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
    // validate
    if (config.backup_name.empty()) {
        return Status(EMPTY_FILENAME, "备份文件名为空");
    }
    if (config.is_encrypt && config.password.empty()) {
        return Status(PASSWORD_ERROR, "密码为空");
    }
    if (config.target_dir.empty()) {
        return Status(EMPTY_FILEPATH, "打包路径为空");
    }
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
    packfile_ofs.close();

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
    compress_file_ofs.close();
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
    const std::vector<BackupData> &src_path)
{
    // validate
    if (config.backup_name.empty()) {
        return Status(EMPTY_FILENAME, "备份文件名为空");
    }
    if (config.is_encrypt && config.password.empty()) {
        return Status(PASSWORD_ERROR, "密码为空");
    }
    if (config.target_dir.empty()) {
        return Status(EMPTY_FILEPATH, "打包路径为空");
    }
    std::string target_path =
        (Path(config.target_dir) / Path(config.backup_name)).ToString();
    try {
        // 1. 打包文件
        FileTree filetree;
        for (const auto &[src, dest, is_partly] : src_path) {
            CheckFilePermission(src, READ);
            filetree.PackFileAdd(src, dest, false, is_partly);
        }
        BackupTreeDetail(
            target_path, filetree, config.is_encrypt, config.password);
    } catch (const Status &s) {
        return s;
    }
    return {OK, ""};
}

void BackUpImpl::BackupTreeDetail(
    const std::string &target_path,
    backup::FileTree &filetree,
    bool is_encrypt,
    const std::string &password)
{
    FilesCleaner cleaner;

    std::string pack_path = target_path + "_pack_temp";
    std::ofstream packfile_ofs(pack_path, std::ios::binary);
    if (!packfile_ofs.is_open()) {
        throw Status{NO_PERMISSION, "无法写入文件" + pack_path};
    }
    filetree.FullDump(packfile_ofs);
    packfile_ofs.close();
    cleaner.AddFile(pack_path);

    // 往backup file写入header信息
    std::ofstream target_file_ofs(target_path, std::ios::binary);
    BackupHeader header;
    header.is_encrypt = is_encrypt;
    header.Dump(target_file_ofs);

    // 2. 压缩文件
    std::ifstream packfile_ifs(pack_path, std::ios::binary);
    if (!packfile_ifs.is_open()) {
        throw Status{NOT_EXIST, "文件" + pack_path + "不存在"};
    }
    std::string compress_path(target_path + "_compress_temp");
    std::ofstream compress_file_ofs;
    if (is_encrypt) {
        compress_file_ofs.open(compress_path, std::ios::binary);
        cleaner.AddFile(compress_path);
    } else {
        compress_file_ofs = std::move(target_file_ofs);
    }
    Compress compress(packfile_ifs, compress_file_ofs);
    compress.CompressFile();
    compress_file_ofs.close();
    // 3. 加密文件
    if (is_encrypt) {
        std::ifstream compress_file_ifs(compress_path, std::ios::binary);
        Encrypt enc(compress_file_ifs, target_file_ofs);
        enc.AES_encrypt_file(password);
    }
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
        if (!ifs.is_open()) {
            throw Status{NOT_EXIST, "无法打开文件" + pack_path};
        }
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

Status BackUpImpl::RestoreBatch(
    const std::string &backup_path,
    const std::vector<std::string> &pack_paths,
    const std::string &target_dir,
    const std::string &password)
{
    FilesCleaner cleaner;
    try {
        std::string uncompress_path = RecoverToPackFile(backup_path, password);
        cleaner.AddFile(uncompress_path);
        std::ifstream ifs(uncompress_path, std::ios::binary);
        if (!ifs.is_open()) {
            throw Status{NOT_EXIST, "无法恢复解压/解密备份文件"};
        }
        FileTree tree;
        tree.Load(ifs);
        for (const std::string &path : pack_paths) {
            tree.Recover(path, ifs, target_dir);
        }

    } catch (const Status &s) {
        return s;
    }
    return Status{OK, ""};
}

Status BackUpImpl::ReBackupFile(
    const std::string &backup_path,
    bool isEncrypt,
    const std::string &password)
{
    FilesCleaner cleaner;
    try {
        std::string uncompress_path = RecoverToPackFile(backup_path, password);
        cleaner.AddFile(uncompress_path);
        std::ifstream ifs(uncompress_path, std::ios::binary);
        if (!ifs.is_open()) {
            throw Status{NOT_EXIST, "无法恢复解压/解密备份文件"};
        }
        FileTree recover_tree;
        recover_tree.Load(ifs);
        // 根据文件树，找到源路径，全部重新备份一遍
        FileTree backup_tree;
        ReBackupFile(recover_tree.GetRootNode(), backup_tree, "");
        std::string backup_new;
        for (int i = 0; i < INT_MAX; i++) {
            backup_new = backup_path + '_' + std::to_string(i);
            if (Access(backup_new, EXIST) == OK) continue;
            if (RenameFile(backup_path, backup_new))
                break;
            else
                throw Status{errno, backup_make_error_code(errno).message()};
        }
        BackupTreeDetail(backup_path, backup_tree, isEncrypt, password);
    } catch (const Status &s) {
        return s;
    }
    return Status(OK, "");
}

BackUpImpl::BackUpImpl()
{
    Config *config = Config::getInstance();
    temp_path = config->getConfigData("temp_path");
    MakeDir(temp_path, 0777); // 创建临时文件夹（不存在时创建）
    // 确保访问权限
    CheckFilePermission(temp_path, READ | WRITE);
}

std::string BackUpImpl::RecoverToPackFile(
    const std::string &backup_path,
    const std::string &password)
{
    CheckFilePermission(backup_path, READ);
    if (Path(backup_path).GetFileType() != FileType::REG) {
        throw Status{ERROR, backup_path + "不是备份文件！"};
    }
    std::ifstream ifs(backup_path, std::ios::binary);
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
    FilesCleaner cleaner;
    std::string temp_prefix = temp_path + '/' + Path(backup_path).FileName();
    auto [st, is_enc] = isEncrypted(backup_path);
    if (st.code != OK) throw st;

    if (!is_enc) {
        std::string uncompress_path = temp_prefix + "_uncompress_temp";
        std::ofstream ofs(uncompress_path, std::ios::binary);
        if (!ofs.is_open()) {
            throw Status{NO_PERMISSION, "无法创建文件" + uncompress_path};
        }
        Compress comp(ifs, ofs);
        comp.DecompressFile();
        return uncompress_path;
    } else {
        std::string decrypted_path = temp_prefix + "_decrypted_temp";
        std::ofstream ofs(decrypted_path, std::ios::binary);
        if (!ofs.is_open()) {
            throw Status{NO_PERMISSION, "无法创建文件" + decrypted_path};
        }
        Encrypt enc(ifs, ofs);
        bool dec_ret = enc.AES_decrypt_file(password);
        ofs.close();
        cleaner.AddFile(decrypted_path);
        if (!dec_ret) { throw Status{PASSWORD_ERROR, "密码错误"}; }
        std::string uncompress_path = temp_prefix + "_uncompress_temp";
        std::ifstream decrypted_file_ifs(decrypted_path, std::ios::binary);
        std::ofstream uncompress_file_ofs(uncompress_path, std::ios::binary);
        if (!uncompress_file_ofs.is_open()) {
            throw Status{NO_PERMISSION, "无法创建文件" + uncompress_path};
        }
        Compress comp(decrypted_file_ifs, uncompress_file_ofs);
        comp.DecompressFile();
        return uncompress_path;
    }
}

void BackUpImpl::CheckFilePermission(const std::string &path, int permissions)
{
    ErrorCode ret = Access(path, permissions);
    if (ret != OK) {
        if (ret == NOT_EXIST) {
            throw Status{NOT_EXIST, "文件/文件夹" + path + "不存在"};

        } else if (ret == NO_PERMISSION) {
            throw Status{
                NO_PERMISSION, "没有对文件/文件夹" + path + "的访问权限"};
        }
    }
}
void BackUpImpl::ReBackupFile(
    std::shared_ptr<FileNode> root,
    FileTree &tree,
    std::string pack_dir)
{
    if (!root->meta_.name.empty()) {
        pack_dir = pack_dir + root->meta_.name + "/";
    }
    for (auto &[filename, node] : root->children_) {
        auto &meta = node->meta_;
        try {
            CheckFilePermission(meta.origin_path, READ);
        } catch (const Status &s) {
            if (s.code != OK) { continue; }
        }
        tree.PackFileAdd(
            meta.origin_path, pack_dir, false, meta.is_partly_check);

        if (meta.is_directory && !meta.is_partly_check) {
            // 添加新增的文件
            auto list = GetFilesFromDir(meta.origin_path);
            std::string cur_pack_dir = pack_dir + meta.name + "/";
            for (auto &filename : list) {
                // 可以直接递归添加文件
                tree.PackFileAdd(
                    (Path(meta.origin_path) / filename).ToString(),
                    cur_pack_dir,
                    true,
                    false);
            }
        } else if (!node->children_.empty()) {
            ReBackupFile(node, tree, pack_dir);
        }
    }
}
} // namespace backup
