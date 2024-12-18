#include <gtest/gtest.h>
#define private public
#include "file_tree.h"

using namespace backup;

void CheckOneFile(std::shared_ptr<FileNode> node, std::string pack_path,
                  std::string filename, std::string origin_path,
                  bool is_directory, FileType type, uint64_t size,
                  bool is_linked_to) {
  EXPECT_EQ(node->meta_.pack_path, pack_path);
  EXPECT_EQ(node->meta_.name, filename);
  EXPECT_EQ(node->meta_.origin_path, origin_path);
  EXPECT_EQ(node->meta_.is_directory, is_directory);
  EXPECT_EQ(node->meta_.type, static_cast<uint8_t>(type));
  EXPECT_EQ(node->meta_.size, size);
  EXPECT_EQ(node->meta_.is_linked_to, is_linked_to);
}

TEST(FileTreeTest, LocateAndCreateDir) {
  FileTree tree;
  auto node = tree.LocateAndCreateDir("");
  EXPECT_EQ(node->meta_.pack_path, "");
  EXPECT_EQ(node.get(), tree.root_.get());
  node = tree.LocateAndCreateDir("dir");
  EXPECT_EQ(node->meta_.pack_path, "dir");
  node = tree.LocateAndCreateDir("dir/dir1/dir2");
  EXPECT_EQ(node->meta_.pack_path, "dir/dir1/dir2");
  EXPECT_EQ(tree.root_->children_["dir"]->children_["dir1"]->meta_.pack_path,
            "dir/dir1");

  node = tree.LocateNode("");
  EXPECT_EQ(node.get(), tree.root_.get());
  node = tree.LocateNode("dir");
  EXPECT_EQ(node.get(), tree.root_->children_["dir"].get());
  node = tree.LocateNode("dir/dir1");
  EXPECT_EQ(node.get(), tree.root_->children_["dir"]->children_["dir1"].get());
  node = tree.LocateNode("dir/dir/dddd/ccc");
  EXPECT_EQ(node.get(), nullptr);
}

TEST(FileTreeTest, PackFileAdd) {
  ::system("rm -rf dir");
  ::system("mkdir dir && touch dir/f1 && touch dir/f2 && mkdir dir/dir");
  FileTree tree;
  Path src = "dir";
  tree.PackFileAdd(src, "/");
  EXPECT_NE(tree.root_->children_.find("dir"), tree.root_->children_.end());
  auto root = tree.root_->children_["dir"];
  src = src.ToFullPath();
  CheckOneFile(root, "dir", "dir", src.ToString(), true, FileType::DIR, 4096,
               false);
  auto cur = root->children_["f1"];
  CheckOneFile(cur, "dir/f1", "f1", (src / Path("f1")).ToString(), false,
               FileType::REG, 0, false);
  cur = root->children_["f2"];
  CheckOneFile(cur, "dir/f2", "f2", (src / Path("f2")).ToString(), false,
               FileType::REG, 0, false);
  cur = root->children_["dir"];
  CheckOneFile(cur, "dir/dir", "dir", (src / Path("dir")).ToString(), true,
               FileType::DIR, 4096, false);
  EXPECT_EQ(root->children_.size(), 3);
  EXPECT_EQ(tree.count_, 4);

  tree.PackFileAdd(src, "dir2");
  root = tree.root_->children_["dir2"];
  // 默认创建一个文件夹
  CheckOneFile(root, "dir2", "dir2", "", true, FileType::DIR, 0, false);
  root = root->children_["dir"];
  CheckOneFile(root, "dir2/dir", "dir", src.ToString(), true, FileType::DIR,
               4096, false);
  cur = root->children_["f1"];
  CheckOneFile(cur, "dir2/dir/f1", "f1", (src / Path("f1")).ToString(), false,
               FileType::REG, 0, false);
  cur = root->children_["dir"];
  CheckOneFile(cur, "dir2/dir/dir", "dir", (src / Path("dir")).ToString(), true,
               FileType::DIR, 4096, false);
  ::system("rm -rf dir");

  // 测试fifo
  ::system("rm -f hello && mkfifo hello");
  src = Path("hello").ToFullPath();
  tree.PackFileAdd(src, "");
  cur = tree.root_->children_["hello"];
  CheckOneFile(cur, "hello", "hello", src.ToString(), false, FileType::FIFO, 0,
               false);

  // 测试软链接
  ::system("rm -f f1 f2 && touch f1 && ln -s f1 f2");
  src = Path("f2").ToFullPath();
  tree.PackFileAdd(src, "");
  cur = tree.root_->children_["f2"];
  CheckOneFile(cur, "f2", "f2", src.ToString(), false, FileType::FLNK, 2,
               false);
  EXPECT_EQ(cur->meta_.link_to_path, "f1");
  Path linkto = Path("f1").ToFullPath();
  EXPECT_EQ(cur->meta_.link_to_full_path, linkto.ToString());

  cur = tree.file_be_linked_[linkto.ToString()];
  // 注意：这里的pack_path没有意义
  CheckOneFile(cur, "f1", "f1", linkto.ToString(), false, FileType::REG, 0,
               true);
  ::system("rm -f f1 f2 hello");
}

TEST(FileTreeTest, FileTreeDump) {
  ::system("echo \"hello world\" > f1");
  FileTree tree;
  tree.PackFileAdd("f1", "");

  std::ofstream ofs("output", std::ios::binary);
  // tree.FullDump(ofs);
  // tree.DumpMeta();
  ofs.flush();
  ofs.close();
  ::system("rm -rf f1");
}