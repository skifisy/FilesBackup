////
// @file pack.h
// @brief
// 文件打包
//
// @author yuhaoze
// @email 743544510@qq.com
//

#pragma once
#include <stat.h>
#include <time.h>
#include <types.h>

#include <memory>
#include <string>
#include <vector>

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

void DumpString(const std::string& str, ofstream& ofs);
void DumpInt32(uint32_t num, ofstream& ofs);
void DumpInt64(uint64_t num, ofstream& ofs);

void LoadString(std::string& str, ifstream& ifs);
void LoadInt32(uint32_t &num, ifstream& ifs);
void LoadInt64(uint64_t & num, ifstream& ifs);

struct BackupFileHeader {
  char magic[5];       // 魔数标识符，例如 "BKUP"
  uint32_t version;    // 备份文件格式的版本号
  uint64_t timestamp;  // 备份的创建时间戳
  uint32_t backupType;  // 备份类型：0表示全量备份，1表示增量备份
  uint64_t hash;        // 当前备份的唯一标识
  uint64_t metadataOffset;  // 元数据区域的起始偏移量
  uint64_t fileDataOffset;  // 文件内容区域的起始偏移量
  uint64_t fileCount;       // 文件数量（包含目录）
  void Dump(ofstream& ofs);
  void Load(ifstream& ifs);
};

struct FileMetadata {
  // path格式：不含开始的/，结尾不含/，不包含.
  std::string path;    // 文件或文件夹的相对路径（含文件名）
  std::string name;    // 文件名
  bool isDirectory;    // 是否为文件夹
  uint64_t size;       // 文件大小（如果是文件夹则为0）
  mode_t permissions;  // 文件权限
  time_t modTime;      // 最后修改时间
  time_t accessTime;   // 最后访问时间
  time_t createTime;   // 文件创建时间
  bool isLinkTo;       // 是否为软链接文件指向的文件
  uint64_t hash;  // 文件内容的哈希值（用于增量备份时检测修改）
  uint64_t dataOffset;  // 文件内容在文件内容区的起始偏移量（如果是文件）
};

// 文件树结构
class FileNode {
 private:
  FileMetadata meta;
  std::vector<std::shared_ptr<FileNode*>> children;
};


