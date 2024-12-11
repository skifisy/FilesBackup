////
// @file file_tree.h
// @brief
// 文件树，用于支持pack
// 用于一个打包文件的内存表示
//
// @author yuhaoze
// @email 743544510@qq.com
//
#pragma once
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "file_meta.h"
namespace backup {
// 文件树节点
class FileNode {
 public:
  FileNode() = default;
  FileNode(std::ifstream& ifs);
  ~FileNode() = default;
  FileMetadata meta_;

  std::unordered_map<std::string, std::shared_ptr<FileNode>> children_;
  // std::vector<std::shared_ptr<FileNode>> children_;
};

// 文件树（表示需要打包的所有文件）
// 两种获取形式
// 1. 打包过程：根据文件路径，搜索所有需要打包的文件
// 2. 解包过程：根据打包文件，进行文件解析，构造tree
class FileTree {
 public:
  FileTree() : root_(std::make_shared<FileNode>()) {}
  ~FileTree() = default;
  // 在文件树中添加文件
  void PackFileAdd(const Path& path);

  // 根据打包文件解析
  void UnPackFileAdd(std::ifstream& ifs);
  uint64_t Size() { return sizeof(FileMetadata) * count_; }

 private:
  void PackFileAdd(const Path& path, std::shared_ptr<FileNode> file_node);
  /**
   * @param file_node 软链接文件的文件节点
   */
  void SaveBeLinked(const Path& path, std::shared_ptr<FileNode> file_node);

  std::shared_ptr<FileNode> LocateAndCreateDir(const Path& path);

  std::shared_ptr<FileNode> root_;  // dummy node 虚拟根节点
  std::vector<std::shared_ptr<FileNode>> file_be_linked_;  // 备份被链接的文件
  size_t count_ = 0;  // 文件 和 文件夹个数
};

}  // namespace backup
