////
// @file file_sys.h
// @brief
// 文件系统的封装
//
// @author yuhaoze
// @email 743544510@qq.com
//
#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace backup {
struct FileMetadata;

namespace fs = std::filesystem;

enum class FileType : uint8_t
{
    REG,     // 普通文件
    DIR,     // 目录
    SOCK,    // socket文件
    CHR,     // 字符文件
    FIFO,    // 管道文件
    BLK,     // 块文件
    FLNK,    // 软链接
    UNKNOWN, // 未知文件
    NOTEXIST // 文件不存在
};

class Path
{
  public:
    // TODO: 处理/结尾路径，处理相对路径&全路径
    Path() = default;

    // 面对左值发生copy，面对右值移动
    Path(std::string path);
    // Path(std::string&& p);
    Path(const Path &path) = default;
    Path(Path &&p) noexcept;
    Path(const char *path)
        : Path(std::string(path))
    {}
    Path &operator=(const Path &path);
    Path &operator=(Path &&path) noexcept;

    ~Path() {};
    // 文件是否存在
    bool IsExist() const;
    bool IsDirectory() const;
    // 是否为相对路径
    bool IsRelative() const;
    // 是否为普通文件
    bool IsRegularFile() const;
    // 获取文件类型
    FileType GetFileType() const;
    std::string ToString() const;
    std::string FileName() const;
    Path ParentPath() const;
    Path &ReplaceFileName(const std::string &file_name);
    // 相对路径转换为绝对路径
    Path ToFullPath() const;
    // 连接路径
    Path operator/(const Path &other) const;
    std::vector<std::string> SplitPath() const;

  private:
    friend bool RemoveFile(const Path &path) noexcept;
    friend bool RemoveAll(const Path &path);

    Path(const fs::path &p);
    Path(fs::path &&p) noexcept;
    fs::path path_;
};

/**
 * @brief 返回dir中的所有文件
 * @return 文件名的vector
 */
std::vector<Path> GetFilesFromDir(const Path &path);

// 获取软链接指向的文件路径
Path GetFileLinkTo(const std::string &path);

// 获取当前工作路径
Path GetCurPath();

/**
 * @brief 保存文件的元信息，将元信息 meta 保存到 target中
 * 元信息包括：
 * 1. 时间
 * 2. 权限
 * 3. 拥有者/拥有组
 */
bool SaveFileMetaData(const FileMetadata &meta, const std::string &target);

bool MakeDir(const std::string &dirname, mode_t mode);

bool Link(const std::string &to, const std::string &from);

bool SymLink(const std::string &to, const std::string &from);

bool MakeFifo(const std::string &filename, mode_t mode);

bool RemoveFile(const Path &path) noexcept;

bool RemoveAll(const Path &path);

std::string UidToString(uid_t uid);

} // namespace backup
