////
// @file file_sys.h
// @brief
// 文件系统的封装
//
// @author yuhaoze
// @email 743544510@qq.com
//
#pragma once
#include <sys/stat.h>

#include <filesystem>
#include <string>
namespace fs = std::filesystem;

enum class FileType : uint8_t {
  REG,
  DIR,
  SOCK,
  CHR,
  FIFO,
  BLK,
  FLNK,
  UNKNOWN,
  NOTEXIST
};

class Path {
 public:
  // TODO: 处理/结尾路径，处理相对路径&全路径
  Path(std::string path) {
    // /结尾路径：/foo/
    if (path.back() == '/') path.pop_back();
    // TODO: 相对路径处理？(软链接指向的路径)
    path_ = path;
  }
  ~Path(){};
  bool IsExist() { return fs::exists(path_); }
  bool IsDirectory() { return fs::is_directory(path_); }
  FileType GetFileType() {
    struct stat st;
    if (lstat(path_.string().c_str(), &st) != 0) {
      return FileType::NOTEXIST;
    }
  }

  std::string ToString() { return path_.string(); }

 private:
  fs::path path_;
};

