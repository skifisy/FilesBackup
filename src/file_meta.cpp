#include <cstring>

#include "file_meta.h"
namespace backup {

void DumpString(const std::string &str, std::ofstream &ofs) {
  uint32_t length = str.size();
  ofs.write(reinterpret_cast<const char *>(&length), sizeof(length));
  ofs.write(str.c_str(), length);
  // std::cout << "dumpString" << std::endl;
}

void LoadString(std::string &str, std::ifstream &ifs) {
  uint32_t length;
  ifs.read(reinterpret_cast<char *>(&length), sizeof(length));
  str.resize(length);
  ifs.read(&str[0], length);
}

void DumpVar(bool t, std::ofstream &ofs) {
  uint8_t b = t ? 1 : 0;
  DumpVar(b, ofs);
}

void LoadVar(bool &t, std::ifstream &ifs) {
  uint8_t b;
  LoadVar(b, ifs);
  t = b == 1 ? true : false;
}

void BackupFileHeader::Dump(std::ofstream &ofs) const {
  DumpString(magic, ofs);
  DumpVar(version, ofs);
  DumpVar(timestamp, ofs);
  DumpVar(backup_type, ofs);
  // DumpVar(hash, ofs);
  DumpVar(metadata_offset, ofs);
  DumpVar(linkto_metadata_offset, ofs);
  DumpVar(file_data_offset, ofs);
  DumpVar(addition_back_offset, ofs);
  DumpVar(footer_offset, ofs);
  DumpVar(file_count, ofs);
  DumpVar(linkto_count, ofs);
}

void BackupFileHeader::Load(std::ifstream &ifs) {
  std::string str;
  LoadString(str, ifs);
  strncpy(magic, str.c_str(), sizeof(magic) - 1);
  LoadVar(version, ifs);
  LoadVar(timestamp, ifs);
  LoadVar(backup_type, ifs);
  // LoadVar(hash, ifs);
  LoadVar(metadata_offset, ifs);
  LoadVar(linkto_metadata_offset, ifs);
  LoadVar(file_data_offset, ifs);
  LoadVar(addition_back_offset, ifs);
  LoadVar(footer_offset, ifs);
  LoadVar(file_count, ifs);
  LoadVar(linkto_count, ifs);
}

void FileMetadata::Dump(std::ofstream &ofs) const {
  DumpString(pack_path, ofs);
  DumpString(name, ofs);
  DumpString(origin_path, ofs);
  DumpVar(is_directory, ofs);
  DumpVar(type, ofs);
  DumpVar(size, ofs);
  DumpVar(permissions, ofs);
  DumpVar(mod_time, ofs);
  DumpVar(access_time, ofs);
  DumpVar(uid, ofs);
  DumpVar(gid, ofs);
  DumpVar(is_linked_to, ofs);
  DumpString(link_to_path, ofs);
  DumpString(link_to_full_path, ofs);
  // DumpString(hash, ofs);
  DumpVar(link_num, ofs);
  DumpVar(ino, ofs);
  DumpVar(data_offset, ofs);
}

void FileMetadata::Load(std::ifstream &ifs) {
  LoadString(pack_path, ifs);
  LoadString(name, ifs);
  LoadString(origin_path, ifs);
  LoadVar(is_directory, ifs);
  LoadVar(type, ifs);
  LoadVar(size, ifs);
  LoadVar(permissions, ifs);
  LoadVar(mod_time, ifs);
  LoadVar(access_time, ifs);
  LoadVar(uid, ifs);
  LoadVar(gid, ifs);
  LoadVar(is_linked_to, ifs);
  LoadString(link_to_path, ifs);
  LoadString(link_to_full_path, ifs);
  // LoadString(hash, ifs);
  LoadVar(link_num, ifs);
  LoadVar(ino, ifs);
  LoadVar(data_offset, ifs);
}

void FileMetadata::SetFromPath(const Path &src, const std::string &dest) {
  struct stat st {};
  if (lstat(src.ToString().c_str(), &st) != 0) {
    throw std::runtime_error("not find file " + src.ToString());
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
  link_num = st.st_nlink;  // 硬链接数
  ino = st.st_ino;
  // TODO: hash
}

}  // namespace backup
