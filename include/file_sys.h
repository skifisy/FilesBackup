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
namespace fs = std::filesystem;

enum class FileType : uint8_t {
  REG,      // 普通文件
  DIR,      // 目录
  SOCK,     // socket文件
  CHR,      // 字符文件
  FIFO,     // 管道文件
  BLK,      // 块文件
  FLNK,     // 软链接
  UNKNOWN,  // 未知文件
  NOTEXIST  // 文件不存在
};

class Path {
 public:
  // TODO: 处理/结尾路径，处理相对路径&全路径
  Path(std::string path);
  Path(const Path& path) = default;

  ~Path(){};
  bool IsExist() const;
  bool IsDirectory() const;
  // 是否为相对路径
  bool IsRelative() const;
  bool IsRegular() const;
  FileType GetFileType() const;
  std::string ToString() const;
  std::string FileName() const;
  Path& ReplaceFileName(const std::string& file_name);
  // 连接路径
  Path operator/(const Path& other) const;

 private:
  Path(const fs::path& p);
  fs::path path_;
};

std::vector<Path> GetFilesFromDir(const Path& path);

// 获取软链接指向的文件路径
Path GetFileLinkTo(const std::string& path);