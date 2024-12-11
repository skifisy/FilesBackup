#include "file_tree.h"
namespace backup {

FileNode::FileNode(std::ifstream &ifs) { meta_.Load(ifs); }

void FileTree::PackFileAdd(const Path &path) {
  // TODO:
  // 上面的需求，这个path可能在某个路径下，也可能是独立的一个file，也可以【乱序】
  // 但是都要求都打包进来
  // require: path必须为相对路径

  // TODO:
  // 所以这里就需要根据path来定位，如果不存在dir，可能要提前创建dir。所以下面dir的处理也要修改
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
        file_node->children_.emplace(node->meta_.name, node);
      }
      break;
    case FileType::FLNK:
      // 软链接文件保存软链接指向的文件
      SaveBeLinked(path, file_node);
      break;
    default:
      break;
  }
  root_->children_.emplace(file_node->meta_.name, file_node);
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

std::shared_ptr<FileNode> FileTree::LocateAndCreateDir(const Path &path) {
  std::vector<std::string> paths = path.SplitPath();
  for (auto dir : paths) {
  }
  return std::shared_ptr<FileNode>();
}

}  // namespace backup
