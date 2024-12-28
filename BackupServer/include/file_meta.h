////
// @file file_meta.h
// @brief
// 文件元信息，用于文件的磁盘表示
//
// @author yuhaoze
// @email 743544510@qq.com
//
#pragma once

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <cstring>
#include <fstream>
#include <string>

#include "file_sys.h"

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

namespace backup {
#define BACK_MAGIC "BKP"

enum RegularTimeType
{
    none = 0,
    every_day = 1,
    every_week = 2
};

struct BackupFileHeader
{
    char magic[4];    // 标识符，表示文件类型 "BKU"
    uint32_t version; // 备份文件格式的版本号
    time_t timestamp; // 备份的创建时间戳
    uint8_t backup_type; // 备份类型：0表示全量备份，1表示增量备份
    // uint64_t hash;         // 当前备份的唯一标识
    uint64_t metadata_offset;        // 元数据区域的起始偏移量
    uint64_t linkto_metadata_offset; // 链接源文件元数据的起始偏移
    uint64_t file_data_offset;       // 文件内容区域的起始偏移量
    uint64_t addition_back_offset;   // 增量备份区域的起始偏移量
    uint64_t footer_offset;          // 文件尾的偏移量
    uint64_t file_count;             // 文件数量（包含目录）
    uint64_t linkto_count;           // 链接源文件数量

    BackupFileHeader()
    {
        memset(this, 0, sizeof(BackupFileHeader));
        strcpy(magic, BACK_MAGIC);
    }
    static constexpr size_t Size()
    {
        // 注意加上string的4字节长度
        return sizeof(magic) - 1 + sizeof(uint32_t) + sizeof(version) +
               sizeof(timestamp) + sizeof(backup_type) +
               sizeof(metadata_offset) + sizeof(linkto_metadata_offset) +
               sizeof(file_data_offset) + sizeof(addition_back_offset) +
               sizeof(footer_offset) + sizeof(file_count) +
               sizeof(linkto_count);
    }
    size_t Dump(std::ofstream &ofs) const;
    size_t Load(std::ifstream &ifs);
};

struct FileMetadata
{
    // path格式：不含开始的/，结尾不含/
    // 相对路径，相对于打包文件
    std::string pack_path; // 文件或文件夹的打包路径（含文件名）
    std::string name;      // 文件名
    std::string origin_path;  // 源路径（用于验证）
    bool is_directory;        // 是否为文件夹
    uint8_t type;             // 文件类型
    uint64_t size;            // 文件大小（如果是文件夹则为0）
    mode_t permissions;       // 文件权限
    time_t mod_time;          // 最后修改时间
    time_t access_time;       // 最后访问时间
    uint32_t uid;             // 用户id
    uint32_t gid;             // 用户组id
    bool is_linked_to;        // 是否为软链接文件指向的文件
    std::string link_to_path; // 软链接指向的文件路径（原始路径）
    std::string link_to_full_path; // 软链接指向的文件->绝对路径
    // std::string hash;  // 文件内容的哈希值（用于增量备份时检测修改）
    uint64_t link_num; // 文件的链接数（硬链接）
    uint64_t ino;      // inode节点号
    uint64_t data_offset = 0; // 文件内容在文件内容区的起始偏移量（如果是文件）
    size_t Size() const
    {
        return sizeof(uint32_t) * 5 // string
               + pack_path.size() + name.size() + origin_path.size() +
               sizeof(uint8_t) * 2 // bool
               + sizeof(type) + sizeof(size) + sizeof(permissions) +
               sizeof(mod_time) + sizeof(access_time) + sizeof(uid) +
               sizeof(gid) + link_to_path.size() + link_to_full_path.size() +
               sizeof(link_num) + sizeof(ino) + sizeof(data_offset);
    }
    size_t Dump(std::ofstream &ofs) const;
    size_t Load(std::ifstream &ifs);
    void SetFromPath(const Path &src, const std::string &dest = "");
    void SetDataOffset(uint64_t offset) { data_offset = offset; }
};

size_t DumpString(const std::string &str, std::ofstream &ofs);

size_t LoadString(std::string &str, std::ifstream &ifs);

template <typename T>
size_t DumpVar(const T &t, std::ofstream &ofs)
{
    ofs.write(reinterpret_cast<const char *>(&t), sizeof(t));
    return sizeof(t);
}

size_t DumpVar(bool t, std::ofstream &ofs);

template <typename T>
size_t LoadVar(T &t, std::ifstream &ifs)
{
    ifs.read(reinterpret_cast<char *>(&t), sizeof(t));
    return sizeof(t);
}

size_t LoadVar(bool &t, std::ifstream &ifs);

} // namespace backup
