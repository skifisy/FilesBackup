

#pragma once
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "file_sys.h"
#include "pack.h"

/**

+----------------------+
| Backup File Header   |  <- 备份文件头部，包含元信息
+----------------------+
| Metadata Section     |  <- 元数据区，记录文件和文件夹的元信息
|   - FileMetadata[1]  |
|   - FileMetadata[2]  |
|   - ...              |
+----------------------+
| File Content Section |  <- 文件内容区，存储文件的实际数据
|   - FileData[1]      |
|   - FileData[2]      |
+----------------------+

 */

struct BackupFileHeader {
  char magic[5];       // 魔数标识符，例如 "BKUP"
  uint32_t version;    // 备份文件格式的版本号
  uint64_t timestamp;  // 备份的创建时间戳
  uint32_t backup_type;  // 备份类型：0表示全量备份，1表示增量备份
  uint64_t hash;         // 当前备份的唯一标识
  uint64_t metadata_offset;   // 元数据区域的起始偏移量
  uint64_t file_data_offset;  // 文件内容区域的起始偏移量
  uint64_t file_count;        // 文件数量（包含目录）
  void Dump(std::ofstream& ofs);
  void Load(std::ifstream& ifs);
};

struct FileMetadata {
  // path格式：不含开始的/，结尾不含/
  // 相对路径，相对于打包文件
  std::string path;    // 文件或文件夹的相对路径（含文件名）
  std::string name;    // 文件名
  bool is_directory;   // 是否为文件夹
  uint8_t type;        // 文件类型
  uint64_t size;       // 文件大小（如果是文件夹则为0）
  mode_t permissions;  // 文件权限
  time_t mod_time;     // 最后修改时间
  time_t access_time;  // 最后访问时间
  bool is_linked_to;   // 是否为软链接文件指向的文件
  std::string link_to_path;  // 软链接指向的文件路径（原始路径）
  std::string hash;  // 文件内容的哈希值（用于增量备份时检测修改）
  uint64_t link_num;  // 文件的链接数
  uint64_t data_offset;  // 文件内容在文件内容区的起始偏移量（如果是文件）
  void Dump(std::ofstream& ofs);
  void Load(std::ifstream& ifs);
  void SetFromPath(const Path& p);
  void SetDataOffset(uint64_t offset) { data_offset = offset; }
};

// 文件树节点
class FileNode {
 public:
  FileNode() = default;
  FileNode(std::ifstream& ifs);
  ~FileNode() = default;
  FileMetadata meta_;
  std::vector<std::shared_ptr<FileNode>> children_;
};

// 文件树（表示需要打包的所有文件）
// 两种获取形式
// 1. 打包过程：根据文件路径，搜索所有需要打包的文件
// 2. 解包过程：根据打包文件，进行文件解析，构造tree
class FileTree {
 public:
  FileTree() = default;
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
  std::vector<std::shared_ptr<FileNode>> file_trees_;
  std::vector<std::shared_ptr<FileNode>> file_be_linked_;  // 备份被链接的文件
  size_t count_ = 0;  // 文件 和 文件夹个数
};

class FilePackImpl : public FilePack {
 public:
  FilePackImpl() = default;
  ~FilePackImpl() = default;
  // 将某个文件/文件夹打包到某个位置上
  void Pack(const std::string& src, const std::string& dest);
  // 将某个打包文件解包到某个位置上
  void UnPack(const std::string& src, const std::string& dest);
  void PackBatch();
  void UnPackBatch();

 private:
  Path path_;
  BackupFileHeader file_header_;
};

void DumpString(const std::string& str, std::ofstream& ofs);

void LoadString(std::string& str, std::ifstream& ifs);

template <typename T>
void DumpVar(const T& t, std::ofstream& ofs) {
  ofs.write(reinterpret_cast<const char*>(&t), sizeof(t));
}

void DumpVar(bool t, std::ofstream& ofs);

template <typename T>
void LoadVar(T& t, std::ifstream& ifs) {
  ifs.read(reinterpret_cast<char*>(&t), sizeof(t));
}

void LoadVar(bool& t, std::ifstream& ifs);