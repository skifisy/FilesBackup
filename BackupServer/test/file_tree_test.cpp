#include <gtest/gtest.h>
#define private public
#include <cstring>

#include "file_tree.h"
using namespace backup;

void CheckOneFile(
    std::shared_ptr<FileNode> node,
    std::string pack_path,
    std::string filename,
    std::string origin_path,
    bool is_directory,
    FileType type,
    uint64_t size,
    bool is_linked_to)
{
    EXPECT_EQ(node->meta_.pack_path, pack_path);
    EXPECT_EQ(node->meta_.name, filename);
    EXPECT_EQ(node->meta_.origin_path, origin_path);
    EXPECT_EQ(node->meta_.is_directory, is_directory);
    EXPECT_EQ(node->meta_.type, static_cast<uint8_t>(type));
    EXPECT_EQ(node->meta_.size, size);
    EXPECT_EQ(node->meta_.is_linked_to, is_linked_to);
}

void CompareNode(std::shared_ptr<FileNode> n1, std::shared_ptr<FileNode> n2)
{
    auto &meta1 = n1->meta_;
    auto &meta2 = n2->meta_;
    EXPECT_EQ(meta1.pack_path, meta1.pack_path);
    EXPECT_EQ(meta1.name, meta2.name);
    EXPECT_EQ(meta1.origin_path, meta2.origin_path);
    EXPECT_EQ(meta1.is_directory, meta2.is_directory);
    EXPECT_EQ(meta1.type, meta2.type);
    EXPECT_EQ(meta1.size, meta2.size);
    EXPECT_EQ(meta1.permissions, meta2.permissions);
    EXPECT_EQ(meta1.mod_time, meta2.mod_time);
    EXPECT_EQ(meta1.access_time, meta2.access_time);
    EXPECT_EQ(meta1.uid, meta2.uid);
    EXPECT_EQ(meta1.gid, meta2.gid);
    EXPECT_EQ(meta1.is_linked_to, meta2.is_linked_to);
    EXPECT_EQ(meta1.link_to_path, meta2.link_to_path);
    EXPECT_EQ(meta1.link_to_full_path, meta2.link_to_full_path);
    EXPECT_EQ(meta1.link_num, meta2.link_num);
    EXPECT_EQ(meta1.ino, meta2.ino);
    EXPECT_EQ(meta1.data_offset, meta2.data_offset);
}

void CompareHeader(
    const BackupFileHeader &header1,
    const BackupFileHeader &header2)
{
    EXPECT_EQ(strcmp(header1.magic, header2.magic), 0);
    EXPECT_EQ(header1.version, header2.version);
    EXPECT_EQ(header1.timestamp, header2.timestamp);
    EXPECT_EQ(header1.backup_type, header2.backup_type);
    EXPECT_EQ(header1.metadata_offset, header2.metadata_offset);
    EXPECT_EQ(header1.linkto_metadata_offset, header2.linkto_metadata_offset);
    EXPECT_EQ(header1.file_data_offset, header2.file_data_offset);
    EXPECT_EQ(header1.addition_back_offset, header2.addition_back_offset);
    EXPECT_EQ(header1.footer_offset, header2.footer_offset);
    EXPECT_EQ(header1.file_count, header2.file_count);
    EXPECT_EQ(header1.linkto_count, header2.linkto_count);
}

TEST(FileTreeTest, LocateAndCreateDir)
{
    FileTree tree;
    auto node = tree.LocateAndCreateDir("");
    EXPECT_EQ(node->meta_.pack_path, "");
    EXPECT_EQ(node.get(), tree.root_.get());
    node = tree.LocateAndCreateDir("dir");
    EXPECT_EQ(node->meta_.pack_path, "dir");
    node = tree.LocateAndCreateDir("dir/dir1/dir2");
    EXPECT_EQ(node->meta_.pack_path, "dir/dir1/dir2");
    EXPECT_EQ(
        tree.root_->children_["dir"]->children_["dir1"]->meta_.pack_path,
        "dir/dir1");

    node = tree.LocateNode("");
    EXPECT_EQ(node.get(), tree.root_.get());
    node = tree.LocateNode("dir");
    EXPECT_EQ(node.get(), tree.root_->children_["dir"].get());
    node = tree.LocateNode("dir/dir1");
    EXPECT_EQ(
        node.get(), tree.root_->children_["dir"]->children_["dir1"].get());
    node = tree.LocateNode("dir/dir/dddd/ccc");
    EXPECT_EQ(node.get(), nullptr);
}

TEST(FileTreeTest, PackFileAdd)
{
    EXPECT_EQ(::system("rm -rf dir"), 0);
    EXPECT_EQ(
        ::system("mkdir dir && touch dir/f1 && touch dir/f2 && mkdir dir/dir"),
        0);
    FileTree tree;
    Path src = "dir";
    tree.PackFileAdd(src, "/");
    EXPECT_NE(tree.root_->children_.find("dir"), tree.root_->children_.end());
    auto root = tree.root_->children_["dir"];
    src = src.ToFullPath();
    CheckOneFile(
        root, "dir", "dir", src.ToString(), true, FileType::DIR, 4096, false);
    auto cur = root->children_["f1"];
    CheckOneFile(
        cur,
        "dir/f1",
        "f1",
        (src / Path("f1")).ToString(),
        false,
        FileType::REG,
        0,
        false);
    cur = root->children_["f2"];
    CheckOneFile(
        cur,
        "dir/f2",
        "f2",
        (src / Path("f2")).ToString(),
        false,
        FileType::REG,
        0,
        false);
    cur = root->children_["dir"];
    CheckOneFile(
        cur,
        "dir/dir",
        "dir",
        (src / Path("dir")).ToString(),
        true,
        FileType::DIR,
        4096,
        false);
    EXPECT_EQ(root->children_.size(), 3);
    EXPECT_EQ(tree.count_, 4);

    tree.PackFileAdd(src, "dir2");
    root = tree.root_->children_["dir2"];
    // 默认创建一个文件夹
    CheckOneFile(root, "dir2", "dir2", "", true, FileType::DIR, 0, false);
    root = root->children_["dir"];
    CheckOneFile(
        root,
        "dir2/dir",
        "dir",
        src.ToString(),
        true,
        FileType::DIR,
        4096,
        false);
    cur = root->children_["f1"];
    CheckOneFile(
        cur,
        "dir2/dir/f1",
        "f1",
        (src / Path("f1")).ToString(),
        false,
        FileType::REG,
        0,
        false);
    cur = root->children_["dir"];
    CheckOneFile(
        cur,
        "dir2/dir/dir",
        "dir",
        (src / Path("dir")).ToString(),
        true,
        FileType::DIR,
        4096,
        false);

    EXPECT_EQ(::system("rm -rf dir"), 0);

    // 测试fifo
    EXPECT_EQ(::system("rm -f hello && mkfifo hello"), 0);
    src = Path("hello").ToFullPath();
    tree.PackFileAdd(src, "");
    cur = tree.root_->children_["hello"];
    CheckOneFile(
        cur, "hello", "hello", src.ToString(), false, FileType::FIFO, 0, false);

    // 测试软链接
    EXPECT_EQ(::system("rm -f f1 f2 && touch f1 && ln -s f1 f2"), 0);
    src = Path("f2").ToFullPath();
    tree.PackFileAdd(src, "");
    cur = tree.root_->children_["f2"];
    CheckOneFile(
        cur, "f2", "f2", src.ToString(), false, FileType::FLNK, 2, false);
    EXPECT_EQ(cur->meta_.link_to_path, "f1");
    Path linkto = Path("f1").ToFullPath();
    EXPECT_EQ(cur->meta_.link_to_full_path, linkto.ToString());

    cur = tree.file_be_linked_[linkto.ToString()];
    // 注意：这里的pack_path没有意义
    CheckOneFile(
        cur, "f1", "f1", linkto.ToString(), false, FileType::REG, 0, true);
    EXPECT_EQ(::system("rm -f f1 f2 hello"), 0);
}

TEST(FileTreeTest, FileTreeDump)
{
    EXPECT_EQ(::system("rm -rf for_recover"), 0);
    EXPECT_EQ(::system("echo \"hello world\" > f1"), 0);
    FileTree tree;
    tree.PackFileAdd("f1", "");

    std::ofstream ofs("packfile", std::ios::binary);
    tree.FullDump(ofs);
    ofs.close();
    EXPECT_EQ(tree.header_.metadata_offset, 0x4c);

    FileTree tree2;
    std::ifstream ifs("packfile", std::ios::binary);
    tree2.Load(ifs);
    EXPECT_EQ(tree2.count_, 1);
    CompareHeader(tree.header_, tree2.header_);
    CompareNode(tree.root_->children_["f1"], tree2.root_->children_["f1"]);
    tree2.Recover("", ifs, "for_recover");
    EXPECT_EQ(::system("cmp f1 for_recover/f1"), 0);
    ifs.close();
    EXPECT_EQ(::system("rm -rf f1"), 0);
}

TEST(FileTreeTest, FileTreeDump2)
{
    EXPECT_EQ(::system("rm -rf for_recover dir"), 0);
    EXPECT_EQ(
        ::system(
            "mkdir dir && echo \"hello world\" > dir/f1 && echo \"hello world "
            "file2\" > dir/f2"),
        0);
    EXPECT_EQ(
        ::system("mkfifo dir/fifo && ln -s f1 dir/share && ln dir/f1 dir/f3"),
        0);
    FileTree tree;
    tree.PackFileAdd("dir", "");
    std::ofstream ofs("packfile", std::ios::binary);
    tree.FullDump(ofs);
    ofs.close();
    EXPECT_EQ(tree.header_.metadata_offset, 0x4c);
    FileTree tree2;
    std::ifstream ifs("packfile", std::ios::binary);
    tree2.Load(ifs);
    EXPECT_EQ(tree2.count_, 6);
    CompareHeader(tree.header_, tree2.header_);
    CompareNode(tree.root_->children_["dir"], tree2.root_->children_["dir"]);
    auto dir1 = tree.root_->children_["dir"],
         dir2 = tree2.root_->children_["dir"];
    CompareNode(dir1->children_["f1"], dir2->children_["f1"]);
    CompareNode(dir1->children_["f2"], dir2->children_["f2"]);
    CompareNode(dir1->children_["share"], dir2->children_["share"]);
    CompareNode(dir1->children_["f3"], dir2->children_["f3"]);
    tree2.Recover("", ifs, "for_recover");
    ifs.close();
    EXPECT_EQ(::system("cmp dir/f1 for_recover/dir/f1"), 0);
    EXPECT_EQ(::system("cmp dir/f2 for_recover/dir/f2"), 0);
    EXPECT_EQ(::system("cmp dir/f3 for_recover/dir/f3"), 0);
    EXPECT_EQ(::system("echo \"test test...\" > dir/f1"), 0);
    EXPECT_EQ(::system("cmp for_recover/dir/f1 for_recover/dir/f3"), 0);
    // 检查link，检查file元信息（通过lstat系统调用）
    EXPECT_EQ(::system("rm -rf for_recover dir"), 0);
}

TEST(FileTreeTest, BigFilePackTest)
{
    EXPECT_EQ(::system("rm -rf recover"), 0);
    FileTree tree;
    tree.PackFileAdd("test.jpg", "");

    std::ofstream ofs("packfile", std::ios::binary);
    tree.FullDump(ofs);
    ofs.close();

    // 注意创建另外一个tree，用于打包文件的恢复！
    std::ifstream ifs("packfile", std::ios::binary);
    FileTree tree2;
    tree2.Load(ifs);
    tree2.Recover("test.jpg", ifs, "recover");
    ifs.close();
    EXPECT_EQ(::system("cmp test.jpg recover/test.jpg"), 0);
    EXPECT_EQ(::system("rm -rf recover packfile"), 0);
}

// TODO: Recover目录下文件重名？