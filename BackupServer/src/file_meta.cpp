#include <cstring>

#include "file_meta.h"
#include <iostream>
#include "error_code.h"

namespace backup {

size_t DumpString(const std::string &str, std::ofstream &ofs)
{
    uint32_t length = str.size();
    ofs.write(reinterpret_cast<const char *>(&length), sizeof(length));
    ofs.write(str.c_str(), length);
    return sizeof(length) + length;
}

size_t LoadString(std::string &str, std::ifstream &ifs)
{
    uint32_t length;
    ifs.read(reinterpret_cast<char *>(&length), sizeof(length));
    str.resize(length);
    ifs.read(&str[0], length);
    return sizeof(length) + length;
}

size_t DumpVar(bool t, std::ofstream &ofs)
{
    uint8_t b = t ? 1 : 0;
    return DumpVar(b, ofs);
}

size_t DumpArray(const unsigned char *arr, int size, std::ofstream &ofs)
{
    ofs.write(reinterpret_cast<const char *>(arr), size);
    return size;
}

size_t LoadVar(bool &t, std::ifstream &ifs)
{
    uint8_t b;
    size_t ret = LoadVar(b, ifs);
    t = b == 1 ? true : false;
    return ret;
}

size_t LoadArray(unsigned char *arr, int size, std::ifstream &ifs)
{
    if (ifs.eof()) throw Status{EEOF, "file eof"};
    ifs.read(reinterpret_cast<char *>(arr), size);
    return size;
}

size_t BackupFileHeader::Dump(std::ofstream &ofs) const
{
    return DumpString(magic, ofs) + DumpVar(version, ofs) +
           DumpVar(timestamp, ofs) + DumpVar(backup_type, ofs) +
           // DumpVar(hash, ofs);
           DumpVar(metadata_offset, ofs) +
           DumpVar(linkto_metadata_offset, ofs) +
           DumpVar(file_data_offset, ofs) + DumpVar(addition_back_offset, ofs) +
           DumpVar(footer_offset, ofs) + DumpVar(file_count, ofs) +
           DumpVar(linkto_count, ofs);
}

size_t BackupFileHeader::Load(std::ifstream &ifs)
{
    std::string str;
    size_t ret = 0;
    ret += LoadString(str, ifs);
    strncpy(magic, str.c_str(), sizeof(magic) - 1);
    ret += LoadVar(version, ifs);
    ret += LoadVar(timestamp, ifs);
    ret += LoadVar(backup_type, ifs);
    // LoadVar(hash, ifs);
    ret += LoadVar(metadata_offset, ifs);
    ret += LoadVar(linkto_metadata_offset, ifs);
    ret += LoadVar(file_data_offset, ifs);
    ret += LoadVar(addition_back_offset, ifs);
    ret += LoadVar(footer_offset, ifs);
    ret += LoadVar(file_count, ifs);
    ret += LoadVar(linkto_count, ifs);
    return ret;
}

size_t FileMetadata::Dump(std::ofstream &ofs) const
{
    size_t ret = 0;

    ret += DumpString(pack_path, ofs);
    ret += DumpString(name, ofs);
    ret += DumpString(origin_path, ofs);
    ret += DumpVar(is_directory, ofs);
    ret += DumpVar(is_partly_check, ofs);
    ret += DumpVar(type, ofs);
    ret += DumpVar(size, ofs);
    ret += DumpVar(permissions, ofs);
    ret += DumpVar(mod_time, ofs);
    ret += DumpVar(access_time, ofs);
    ret += DumpVar(uid, ofs);
    ret += DumpVar(gid, ofs);
    ret += DumpVar(is_linked_to, ofs);
    ret += DumpString(link_to_path, ofs);
    ret += DumpString(link_to_full_path, ofs);
    // if (type == static_cast<int>(FileType::REG))
    ret += DumpArray(hash, SHA256_SIZE, ofs);
    ret += DumpVar(link_num, ofs);
    ret += DumpVar(ino, ofs);
    ret += DumpVar(data_offset, ofs);
    return ret;
}

size_t FileMetadata::Load(std::ifstream &ifs)
{
    size_t ret = 0;
    ret += LoadString(pack_path, ifs);
    ret += LoadString(name, ifs);
    ret += LoadString(origin_path, ifs);
    ret += LoadVar(is_directory, ifs);
    ret += LoadVar(is_partly_check, ifs);
    ret += LoadVar(type, ifs);
    ret += LoadVar(size, ifs);
    ret += LoadVar(permissions, ifs);
    ret += LoadVar(mod_time, ifs);
    ret += LoadVar(access_time, ifs);
    ret += LoadVar(uid, ifs);
    ret += LoadVar(gid, ifs);
    ret += LoadVar(is_linked_to, ifs);
    ret += LoadString(link_to_path, ifs);
    ret += LoadString(link_to_full_path, ifs);
    // if (type == static_cast<int>(FileType::REG))
    ret += LoadArray(hash, SHA256_SIZE, ifs);
    ret += LoadVar(link_num, ifs);
    ret += LoadVar(ino, ifs);
    ret += LoadVar(data_offset, ifs);
    return ret;
}

void FileMetadata::SetFromPath(const Path &src, const std::string &dest)
{
    struct stat st
    {};
    if (lstat(src.ToString().c_str(), &st) != 0) {
        throw Status{NOT_EXIST, "找不到文件 \"" + src.ToString() + '\"'};
    }
    if (dest.empty()) {
        pack_path = src.FileName();
    } else {
        pack_path = dest + "/" + src.FileName();
    }
    name = src.FileName();
    if (src.IsRelative()) {
        origin_path = src.ToFullPath().ToString();
    } else {
        origin_path = src.ToString();
    }
    FileType fileType = src.GetFileType();
    is_directory = (fileType == FileType::DIR);
    this->type = static_cast<uint8_t>(fileType);
    size = st.st_size;
    permissions = st.st_mode;
    mod_time = st.st_mtime;
    access_time = st.st_atime;
    uid = st.st_uid;
    gid = st.st_gid;
    is_linked_to = false;
    link_num = st.st_nlink; // 硬链接数
    ino = st.st_ino;
    if (fileType == FileType::REG) {
        // Attention: 一定不能打开管道文件（会阻塞）
        std::ifstream ifs(src.ToString(), std::ios::binary);

        if (!ifs.is_open() || ifs.bad())
            throw Status{ERROR, "无法打开文件" + src.ToString()};
        if (!compute_file_sha256(
                ifs, reinterpret_cast<unsigned char *>(hash))) {
            throw Status{
                UNABLE_HASH, "文件" + src.ToString() + "无法计算哈希值"};
        }
    }
}

} // namespace backup
