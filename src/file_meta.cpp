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

void BackupFileHeader::Dump(std::ofstream &ofs) {
  DumpString(magic, ofs);
  DumpVar(version, ofs);
  DumpVar(timestamp, ofs);
  DumpVar(backup_type, ofs);
  DumpVar(hash, ofs);
  DumpVar(metadata_offset, ofs);
  DumpVar(file_data_offset, ofs);
  DumpVar(file_count, ofs);
}

void BackupFileHeader::Load(std::ifstream &ifs) {
  std::string str;
  LoadString(str, ifs);
  strncpy(magic, str.c_str(), sizeof(magic) - 1);
  LoadVar(version, ifs);
  LoadVar(timestamp, ifs);
  LoadVar(backup_type, ifs);
  LoadVar(hash, ifs);
  LoadVar(metadata_offset, ifs);
  LoadVar(file_data_offset, ifs);
  LoadVar(file_count, ifs);
}

void FileMetadata::Dump(std::ofstream &ofs) const {
  DumpString(path, ofs);
  DumpString(name, ofs);
  DumpVar(is_directory, ofs);
  DumpVar(type, ofs);
  DumpVar(size, ofs);
  DumpVar(permissions, ofs);
  DumpVar(mod_time, ofs);
  DumpVar(access_time, ofs);
  DumpVar(is_linked_to, ofs);
  DumpString(hash, ofs);
  DumpVar(data_offset, ofs);
}

void FileMetadata::Load(std::ifstream &ifs) {
  LoadString(path, ifs);
  LoadString(name, ifs);
  LoadVar(is_directory, ifs);
  LoadVar(type, ifs);
  LoadVar(size, ifs);
  LoadVar(permissions, ifs);
  LoadVar(mod_time, ifs);
  LoadVar(access_time, ifs);
  LoadVar(is_linked_to, ifs);
  LoadString(hash, ifs);
  LoadVar(data_offset, ifs);
}

void FileMetadata::SetFromPath(const Path &p) {
  struct stat st {};
  if (lstat(p.ToString().c_str(), &st) != 0) {
    throw std::runtime_error("not find file " + p.ToString());
  }
  path = p.ToString();
  name = p.FileName();
  FileType fileType = p.GetFileType();
  is_directory = (fileType == FileType::DIR);
  this->type = static_cast<uint8_t>(fileType);
  size = st.st_size;
  permissions = st.st_mode;
  mod_time = st.st_mtime;
  access_time = st.st_atime;
  is_linked_to = false;
  link_num = st.st_nlink;
  // TODO: hash
}

}  // namespace backup
