#include <dirent.h>
#include <fcntl.h>     // For file descriptor
#include <sys/stat.h>  // For stat, chmod
#include <unistd.h>    // For read, write, link
#include <utime.h>     // For utime

#include <cstring>
#include <iostream>
#include <unordered_map>  // For tracking inodes

#include "backup_impl.h"

void BackUpImpl::Copy(const std::string& src, const std::string& dest) {
  std::unordered_map<ino_t, std::string> ino_map;
  std::vector<std::pair<std::string, std::string>> dir_list;
  CheckSrcDestDir(src, dest);
  CopyFiles(src, dest, ino_map, dir_list, src, dest);
  // 修改目录的元信息
  for (auto dirs : dir_list) {
    CopyFileMetadata(dirs.first, dirs.second);
  }
}

void BackUpImpl::Save(const std::string& src, const std::string& dest) {}

void BackUpImpl::Recover(const std::string& src, const std::string& dest) {}

bool BackUpImpl::IsDirectory(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) != 0) {
    std::cerr << "Failed to get file stat for: " << path << std::endl;
    return false;
  }
  if (S_ISDIR(st.st_mode)) return true;
  return false;
}

bool BackUpImpl::IsExist(const std::string& path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0;
}

bool BackUpImpl::CheckSrcDestDir(const std::string& src,
                                 const std::string& dest) {
  if (!IsExist(src)) {
    std::cerr << "Failed to get file stat for: " << src << std::endl;
    return false;
  }
  if (IsExist(dest)) {
    if (!IsDirectory(dest)) {
      std::cerr << dest << " is not a directory" << std::endl;
      return false;
    }
  } else {
    if (mkdir(dest.c_str(), 0755) != 0) {
      std::cerr << "Failed to creat dir for: " << dest << std::endl;
      return false;
    }
  }
  return true;
}

void BackUpImpl::CopyFiles(
    const std::string& src, const std::string& dest,
    std::unordered_map<ino_t, std::string>& ino_map,
    std::vector<std::pair<std::string, std::string>>& dir_list,
    const std::string& origin_src, const std::string& orgin_dest) {
  struct stat src_stat;
  std::string dest_file = AppendPath(dest, ToFileName(src));
  DIR* dirp = nullptr;
  struct dirent* dir_entry = nullptr;

  lstat(src.c_str(), &src_stat);

  // 判断是否存在硬链接，如果有则直接链接
  if (ino_map.find(src_stat.st_ino) != ino_map.end()) {
    if (link(ino_map[src_stat.st_ino].c_str(), dest_file.c_str()) != 0) {
      std::cerr << "Can not link " << dest_file << " to "
                << ino_map[src_stat.st_ino] << std::endl;
      return;
    }
    CopyFileMetadata(src, dest_file);
    return;
  }
  ino_map[src_stat.st_ino] = dest_file;
  // 判断文件类型，拷贝文件
  switch (src_stat.st_mode & S_IFMT) {
    case S_IFREG:  // 普通文件
      // 拷贝文件
      CopyFileWithMetadata(src, dest_file);
      break;
    case S_IFDIR:  // 目录文件
      // 1. 在dest中创建对应的目录
      mkdir(dest_file.c_str(), src_stat.st_mode);
      dir_list.emplace_back(src, dest_file);
      // 2. 遍历src中的文件，递归调用
      dirp = opendir(src.c_str());
      if (dirp == nullptr) {
        std::cerr << "Can not open dir " << src << std::endl;
      }
      while ((dir_entry = readdir(dirp)) != NULL) {
        if (strcmp(dir_entry->d_name, ".") == 0 ||
            strcmp(dir_entry->d_name, "..") == 0)
          continue;
        CopyFiles(AppendPath(src, dir_entry->d_name), dest_file, ino_map,
                  dir_list, origin_src, orgin_dest);
      }
      closedir(dirp);
      break;
    case S_IFSOCK:  // 套接字文件
      printf("a socket.\n");
      break;
    case S_IFCHR:  // 字符设备
      printf("a character device.\n");
      break;
    case S_IFIFO:  // 管道
      mkfifo(dest_file.c_str(), 0777);
      CopyFileMetadata(src, dest_file);
      break;
    case S_IFBLK:  // 块文件
      printf("a block device.\n");
      break;
    case S_IFLNK:
      CopySymlinkWithMetadata(src, dest_file, origin_src, orgin_dest);
      break;
    default:
      printf("an unknown type.\n");
      break;
  }
}

void BackUpImpl::CopyFileContent(const std::string& src,
                                 const std::string& dest) {
  // 打开源文件
  int src_fd = open(src.c_str(), O_RDONLY);
  if (src_fd < 0) {
    std::cerr << "Can't open file " << src << std::endl;
    return;
  }
  // 创建目标文件
  int dest_fd = open(dest.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (dest_fd < 0) {
    std::cerr << "Can't creat file " << dest << std::endl;
    return;
  }
  // 分块拷贝文件内容
  char buffer[BUFFER_SIZE];
  ssize_t bytesRead;
  while ((bytesRead = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
    if (write(dest_fd, buffer, bytesRead) != bytesRead) {
      std::cerr << "Error writing " << dest << std::endl;
      close(src_fd);
      close(dest_fd);
      return;
    }
  }

  if (bytesRead < 0) {
    std::cerr << "Error reading from " << src << std::endl;
  }
  close(src_fd);
  close(dest_fd);
}

// 时间、权限、拥有者
void BackUpImpl::CopyFileMetadata(const std::string& src,
                                  const std::string& dest) {
  // 修改时间
  struct stat src_stat;
  lstat(src.c_str(), &src_stat);

  struct utimbuf new_times;
  new_times.actime = src_stat.st_atime;   // 设置访问时间
  new_times.modtime = src_stat.st_mtime;  // 设置修改时间

  if (utime(dest.c_str(), &new_times) != 0) {
    std::cerr << "Can not modify time of " << dest << std::endl;
  }

  // 如果是软链接文件，则不修改权限
  if (src_stat.st_mode & S_IFMT != S_IFLNK) {
    // 修改权限
    if (chmod(dest.c_str(), src_stat.st_mode) != 0) {
      std::cerr << "Can not modify permission of " << dest << std::endl;
    }
  }

  // 修改拥有者/组
  chown(dest.c_str(), src_stat.st_uid, src_stat.st_gid);
}

void BackUpImpl::CopyFileWithMetadata(const std::string& src,
                                      const std::string& dest) {
  CopyFileContent(src, dest);
  CopyFileMetadata(src, dest);
}

void BackUpImpl::CopySymlinkWithMetadata(const std::string& src,
                                         const std::string& dest,
                                         const std::string& origin_src,
                                         const std::string& origin_dest) {
  // 用来存放软链接指向的文件路径
  char targetPath[PATH_MAX];
  ssize_t len = readlink(src.c_str(), targetPath, sizeof(targetPath) - 1);

  if (len == -1) {
    perror("Error reading symbolic link");
    return;
  }

  // 读取成功，添加字符串结束符
  targetPath[len] = '\0';
  std::string link_to = targetPath;
  std::size_t pos = link_to.rfind(origin_src);
  // 两类路径：
  // 1. 全局路径 /home/user/filename
  // 2. 相对路径 filename (相对路径则不作修改)
  if (pos != std::string::npos) {
    link_to = origin_dest + link_to.substr(pos + link_to.size());
  }

  if (symlink(link_to.c_str(), dest.c_str()) != 0) {
    std::cerr << "Can not symlink " << dest << " to " << link_to << std::endl;
  }

  // 软链接文件的权限会被忽略
  struct stat src_stat;
  lstat(src.c_str(), &src_stat);

  struct utimbuf new_times;
  new_times.actime = src_stat.st_atime;
  new_times.modtime = src_stat.st_mtime;
  if (utime(dest.c_str(), &new_times) != 0) {
    std::cerr << "Can not modify time of " << dest << std::endl;
  }

  chown(dest.c_str(), src_stat.st_uid, src_stat.st_gid);
}

std::string BackUpImpl::ToFileName(const std::string& path) {
  std::size_t pos = path.rfind('/');
  if (pos != std::string::npos) {
    return path.substr(pos + 1);
  } else
    return path;
}

std::string BackUpImpl::AppendPath(const std::string& prefix,
                                   const std::string& filename) {
  if (prefix.back() == '/') {
    return prefix + filename;
  } else {
    return prefix + '/' + filename;
  }
}
