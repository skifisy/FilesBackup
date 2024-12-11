#include <dirent.h>
#include <fcntl.h>     // For file descriptor
#include <limits.h>    // 包含 PATH_MAX
#include <sys/stat.h>  // For stat, chmod
#include <unistd.h>    // For read, write, link
#include <utime.h>     // For utime

#include <cassert>
#include <cstring>

#include "file_sys.h"
namespace backup {

Path::Path(std::string path) {
  // /结尾路径：/foo/
  if (path.back() == '/') path.pop_back();
  // TODO: 相对路径处理？(软链接指向的路径)
  path_ = path;
}

bool Path::IsExist() const { return fs::exists(path_); }

bool Path::IsDirectory() const { return fs::is_directory(path_); }

bool Path::IsRelative() const {
  std::string p = path_.string();
  if (!p.empty() && *p.begin() == '/') return false;
  return true;
}

bool Path::IsRegular() const {
  FileType type = GetFileType();
  return type == FileType::REG;
}

FileType Path::GetFileType() const {
  struct stat st;
  if (lstat(path_.string().c_str(), &st) != 0) {
    return FileType::NOTEXIST;
  }
  switch (st.st_mode & S_IFMT) {
    case S_IFREG:  // 普通文件
      return FileType::REG;
      break;
    case S_IFDIR:  // 目录文件
      return FileType::DIR;
      break;
    case S_IFSOCK:  // 套接字文件
      return FileType::SOCK;
      break;
    case S_IFCHR:  // 字符设备
      return FileType::CHR;
      break;
    case S_IFIFO:  // 管道
      return FileType::FIFO;
      break;
    case S_IFBLK:  // 块文件
      return FileType::BLK;
      break;
    case S_IFLNK:
      return FileType::FLNK;
      break;
    default:
      return FileType::UNKNOWN;
      printf("an unknown type.\n");
      break;
  }
}

std::string Path::ToString() const { return path_.string(); }

std::string Path::FileName() const { return path_.filename(); }

Path& Path::ReplaceFileName(const std::string& file_name) {
  path_.replace_filename(file_name);
  return *this;
}

Path Path::ToFullPath() const {
  assert(IsRelative());
  return GetCurPath() / *this;
}

Path Path::operator/(const Path& other) const { return path_ / other.path_; }

std::vector<std::string> Path::SplitPath() const {
  std::string p = ToString();
  if (p.empty()) return {};
  std::size_t start = 0, end = std::string::npos;
  std::vector<std::string> str_list;
  while ((end = p.find('/', start)) != std::string::npos) {
    str_list.emplace_back(p.substr(start, end - start));
    start = end + 1;
  }
  str_list.emplace_back(p.substr(start, end));
  return str_list;
}

Path::Path(const fs::path& p) { path_ = p; }

std::vector<Path> GetFilesFromDir(const Path& path) {
  std::vector<Path> result;
  DIR* dirp = nullptr;
  struct dirent* dir_entry = nullptr;
  dirp = opendir(path.ToString().c_str());
  if (dirp == nullptr) {
    throw std::runtime_error("Can not open dir " + path.ToString());
  }
  while ((dir_entry = readdir(dirp)) != nullptr) {
    if (strcmp(dir_entry->d_name, ".") == 0 ||
        strcmp(dir_entry->d_name, "..") == 0)
      continue;
    result.emplace_back(dir_entry->d_name);
  }
  closedir(dirp);
  return result;
}

Path GetFileLinkTo(const std::string& path) {
  // 用来存放软链接指向的文件路径
  char targetPath[PATH_MAX];
  ssize_t len = readlink(path.c_str(), targetPath, sizeof(targetPath) - 1);
  if (len == -1) {
    throw std::runtime_error("Error reading symbolic link");
    return {};
  }
  // 读取成功，添加字符串结束符
  targetPath[len] = '\0';
  return {std::string(targetPath)};
}

Path GetCurPath() {
  char buffer[PATH_MAX];  // 定义一个缓冲区来存放路径

  // 获取当前工作目录
  if (getcwd(buffer, sizeof(buffer)) == nullptr) {
    std::perror("getcwd() error");
  }
  return {std::string(buffer)};
}
}  // namespace backup
