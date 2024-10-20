#include <cstring>
#include <iostream>

#include "file_sys.h"
#include "pack.h"
#include "pack_impl.h"

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

void FileMetadata::Dump(std::ofstream &ofs) {
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
  struct stat st;
  if (lstat(p.ToString().c_str(), &st) != 0) {
    throw std::runtime_error("not find file " + p.ToString());
  }
  path = p.ToString();
  name = p.FileName();
  FileType type = p.GetFileType();
  is_directory = (type == FileType::DIR);
  this->type = static_cast<uint8_t>(type);
  size = st.st_size;
  permissions = st.st_mode;
  mod_time = st.st_mtime;
  access_time = st.st_atime;
  is_linked_to = false;
  link_num = st.st_nlink;
  // TODO: hash
}

void FilePackImpl::Pack(const std::string &src, const std::string &dest) {
  // 1. 遍历src，构建文件树
  // 2. 将文件树打包为一个大文件

  // 思路2：边遍历，边打包（问题：各种data的offset如何计算？）
}

void FilePackImpl::UnPack(const std::string &src, const std::string &dest) {}

void FilePackImpl::PackBatch() {}

void FilePackImpl::UnPackBatch() {}

FileNode::FileNode(std::ifstream &ifs) { meta_.Load(ifs); }

void FileTree::PackFileAdd(const Path &path) {
  // TODO: 上面的需求，这个path可能在某个路径下，也可能是独立的一个file，也可以【乱序】
  // 但是都要求都打包进来
  // require: path必须为相对路径

  // TODO: 所以这里就需要根据path来定位，如果不存在dir，可能要提前创建dir。所以下面dir的处理也要修改
  // 对于重复出现的文件，是否要更新？
  std::shared_ptr<FileNode> file_node = std::make_shared<FileNode>();
  PackFileAdd(path, file_node);
}

void FileTree::PackFileAdd(const Path &path,
                           std::shared_ptr<FileNode> file_node) {
  file_node->meta_.SetFromPath(path);
  std::vector<Path> files_in_dir;
  switch (path.GetFileType()) {
    case FileType::REG:
    case FileType::FIFO:
      break;
    case FileType::DIR:
      // 进入下一级文件夹
      files_in_dir = GetFilesFromDir(path);
      for (Path &file_name : files_in_dir) {
        std::shared_ptr<FileNode> node = std::make_shared<FileNode>();
        PackFileAdd(path / file_name, node);
        file_node->children_.emplace_back(node);
      }
      break;
    case FileType::FLNK:
      // 软链接文件保存软链接指向的文件
      SaveBeLinked(path, file_node);
      break;
    default:
      break;
  }
  file_trees_.emplace_back(file_node);
  count_++;
}

void FileTree::UnPackFileAdd(std::ifstream &ifs) {}

void FileTree::SaveBeLinked(const Path &path,
                            std::shared_ptr<FileNode> file_node) {
  Path p = GetFileLinkTo(path.ToString());
  file_node->meta_.link_to_path = p.ToString();

  Path q = path;
  // 1. ./file/from  -> ../to
  if (p.IsRelative()) {
    p = q.ReplaceFileName(p.ToString());
  }
  // 2. ./file/from  -> /home/file/from/to

  if (p.IsExist() && p.IsRegular()) {
    std::shared_ptr<FileNode> node = std::make_shared<FileNode>();
    node->meta_.SetFromPath(p);
    node->meta_.is_linked_to = true;
    file_be_linked_.emplace_back(node);
  }
  count_++;
}
