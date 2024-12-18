#include <cassert>
#include <iostream>

#include "file_tree.h"
#define BUFFER_SIZE 1024
namespace backup {

FileNode::FileNode(std::ifstream &ifs) { meta_.Load(ifs); }

void FileTree::PackFileAdd(const Path &src, const Path &dest) {
  // assert(!src.IsRelative());
  assert(dest.IsRelative());
  std::shared_ptr<FileNode> file_node = LocateAndCreateDir(dest);
  PackFileAdd(src, file_node);
}

void FileTree::FullDump(std::ofstream &ofs) {
  InitHeader();
  InitMeta();
  InitLinkMeta();
  header_.Dump(ofs);
  DumpMetaAndLinkMeta(ofs);
  DumpData(ofs);
}

void FileTree::Load(std::ifstream &ifs) {
  header_.Load(ifs);
  assert(strcmp(header_.magic, BACK_MAGIC) == 0);
  assert(header_.backup_type == 0);
  count_ = header_.file_count;
  uint64_t linkto_count = header_.linkto_count;
  // 组织为file_tree
  for (size_t i = 0; i < count_; i++) {
    auto node = std::make_shared<FileNode>();
    node->meta_.Load(ifs);
    Path pack_path = node->meta_.pack_path;
    auto parent = LocateAndCreateDir(pack_path.ParentPath());
    // 注意：dir可能提前创建，所以需要判断child中是否已经有了
    auto ret = parent->children_.find(node->meta_.name);
    if (ret == parent->children_.end()) {
      ret->second->meta_ = node->meta_;
    } else {
      parent->children_.emplace(node->meta_.name, node);
    }
  }
  for (size_t i = 0; i < linkto_count; i++) {
    auto node = std::make_shared<FileNode>();
    file_be_linked_.emplace(node->meta_.origin_path, node);
    node->meta_.Load(ifs);
  }
}

void FileTree::Recover(const Path &pack_path, const Path& target_path) {
  std::shared_ptr<FileNode> node = LocateNode(pack_path);
  assert(node != nullptr);
  switch (static_cast<FileType>(node->meta_.type)) {
    case FileType::REG:
      /* code */
      break;
    case FileType::DIR:
      
      break;
    case FileType::FIFO:
      break;
    case FileType::FLNK:
      break;
    default:
      break;
  }
}

void FileTree::PackFileAdd(const Path &src, std::shared_ptr<FileNode> dest) {
  std::shared_ptr<FileNode> cur = nullptr;
  auto ret = dest->children_.find(src.FileName());
  if (ret != dest->children_.end()) {
    // TODO: 文件重名？
    cur = ret->second;
  } else {
    cur = std::make_shared<FileNode>();
    dest->children_.emplace(src.FileName(), cur);
  }
  cur->meta_.SetFromPath(src, dest->meta_.pack_path);
  std::vector<Path> files_in_dir;
  switch (src.GetFileType()) {
    case FileType::REG:
    case FileType::FIFO:
      break;
    case FileType::DIR:
      // 进入下一级文件夹
      files_in_dir = GetFilesFromDir(src);
      for (Path &file_name : files_in_dir) {
        PackFileAdd(src / file_name, cur);
      }
      break;
    case FileType::FLNK:
      // 软链接文件保存软链接指向的文件
      SaveBeLinked(src, cur);
      break;
    default:
      break;
  }
  count_++;
}

// TODO: 多级link，一个软链接指向另一个软链接
void FileTree::SaveBeLinked(const Path &path,
                            std::shared_ptr<FileNode> file_node) {
  // 软连接指向的路径
  Path target_path = GetFileLinkTo(path.ToString());
  file_node->meta_.link_to_path = target_path.ToString();
  if (target_path.IsRelative()) {
    file_node->meta_.link_to_full_path = target_path.ToFullPath().ToString();
  } else {
    file_node->meta_.link_to_full_path = target_path.ToString();
  }
  // 链接目标文件存在，则保存源文件
  if (target_path.IsExist() && target_path.IsRegularFile()) {
    std::shared_ptr<FileNode> node = std::make_shared<FileNode>();
    node->meta_.SetFromPath(target_path);
    node->meta_.is_linked_to = true;
    file_be_linked_.emplace(node->meta_.origin_path, node);
  }
}

void FileTree::InitHeader() {
  header_.version++;
  header_.timestamp = time(NULL);
  header_.backup_type = 0;
  header_.file_count = count_;
  header_.linkto_count = file_be_linked_.size();

  header_.metadata_offset = sizeof(BackupFileHeader);
  header_.linkto_metadata_offset =
      header_.metadata_offset + sizeof(FileMetadata) * count_;
  header_.file_data_offset = header_.linkto_metadata_offset +
                             sizeof(FileMetadata) * header_.linkto_count;

  // Question: 是否要留footer？
  // header.footer_offset = 0;  // 只有数据填完之后才能写footer
}

std::shared_ptr<FileNode> FileTree::LocateAndCreateDir(const Path &path) {
  std::vector<std::string> paths = path.SplitPath();
  std::shared_ptr<FileNode> cur = root_;
  for (const std::string &dir : paths) {
    auto &children = cur->children_;
    auto ret = children.find(dir);
    if (ret != children.end()) {
      cur = ret->second;
    } else {
      // create
      std::shared_ptr<FileNode> new_file = std::make_shared<FileNode>();
      auto pack_path = cur->meta_.pack_path;
      if (pack_path.empty()) {
        new_file->meta_.pack_path = dir;
      } else {
        new_file->meta_.pack_path = pack_path + '/' + dir;
      }
      new_file->meta_.name = dir;
      new_file->meta_.is_directory = true;
      new_file->meta_.type = static_cast<uint8_t>(FileType::DIR);

      children.emplace(dir, new_file);
      cur = new_file;
    }
  }
  return cur;
}

std::shared_ptr<FileNode> FileTree::LocateNode(const Path &path) {
  auto cur = root_;
  for (auto file_name : path.SplitPath()) {
    auto ret = cur->children_.find(file_name);
    if (ret == cur->children_.end()) {
      return nullptr;
    } else {
      cur = ret->second;
    }
  }
  return cur;
}

void FileTree::InitNode(std::shared_ptr<FileNode> root, uint64_t &data_offset) {
  for (auto child : root->children_) {
    auto node = child.second;

    nodes_to_save_.emplace_back(node);
    // node->meta_.Dump(ofs);
    if (!node->children_.empty()) {
      InitNode(node, data_offset);
    }
    if (node->meta_.type == static_cast<uint8_t>(FileType::REG)) {
      auto ret = ino_map_.find(node->meta_.ino);
      if (ret != ino_map_.end()) {
        node->meta_.data_offset = ret->second->meta_.data_offset;
      } else {
        // 在ino_map没有, 并且是普通文件，则创建新的data block
        node->meta_.data_offset = data_offset;
        data_offset += node->meta_.size;
        // files_to_save.emplace_back(node);
      }
    }
  }
}

void FileTree::DumpData(std::ofstream &ofs) {
  char buffer[BUFFER_SIZE];
  uint64_t data_offset = header_.file_data_offset;
  for (auto node : nodes_to_save_) {
    assert(node->meta_.data_offset <= data_offset);
    if (node->meta_.data_offset == data_offset) {
      std::ifstream ifs(node->meta_.origin_path, std::ios::binary);
      if (!ifs) {
        std::cerr << "无法打开文件: " << node->meta_.origin_path << std::endl;
      }
      while (ifs.read(buffer, BUFFER_SIZE)) {
        ofs.write(buffer, ifs.gcount());
      }
      if (ifs.gcount() > 0) {
        ofs.write(buffer, ifs.gcount());
      }
      data_offset += node->meta_.size;
    }
  }

  for (auto &[path, node] : file_be_linked_) {
    assert(node->meta_.data_offset <= data_offset);
    if (node->meta_.data_offset == data_offset) {
      std::ifstream ifs(node->meta_.origin_path, std::ios::binary);
      if (!ifs) {
        std::cerr << "无法打开文件: " << node->meta_.origin_path << std::endl;
      }
      while (ifs.read(buffer, BUFFER_SIZE)) {
        ofs.write(buffer, ifs.gcount());
      }
      if (ifs.gcount() > 0) {
        ofs.write(buffer, ifs.gcount());
      }
      data_offset += node->meta_.size;
    }
  }
  assert(data_offset == header_.footer_offset);
}

void FileTree::InitMeta() {
  if (root_->children_.empty()) {
    return;
  }
  uint64_t data_offset = header_.file_data_offset;
  InitNode(root_, data_offset);
  header_.footer_offset = data_offset;
}

void FileTree::InitLinkMeta() {
  uint64_t data_offset = header_.footer_offset;
  for (auto &[path, node] : file_be_linked_) {
    if (node->meta_.type == static_cast<uint8_t>(FileType::REG)) {
      auto ret = ino_map_.find(node->meta_.ino);
      if (ret != ino_map_.end()) {
        node->meta_.data_offset = ret->second->meta_.data_offset;
      } else {
        // 在ino_map没有, 并且是普通文件，则创建新的data block
        node->meta_.data_offset = data_offset;
        data_offset += node->meta_.size;
        // files_to_save.emplace_back(node);
      }
    }
  }
  header_.footer_offset = data_offset;
}

void FileTree::DumpMetaAndLinkMeta(std::ofstream &ofs) {
  for (auto node : nodes_to_save_) {
    node->meta_.Dump(ofs);
  }
  for (auto &[path, node] : file_be_linked_) {
    node->meta_.Dump(ofs);
  }
}

}  // namespace backup
