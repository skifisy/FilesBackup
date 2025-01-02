////
// @file file_tree.h
// @brief
// 文件树，用于支持pack
// 用于一个打包文件的内存表示
//
// @author hz
// @email skyfishine@163.com
//
#pragma once
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "file_meta.h"
namespace backup {
// 文件树节点
class FileNode
{
  public:
    FileNode() = default;
    FileNode(std::ifstream &ifs);
    ~FileNode() = default;
    FileMetadata meta_;
    // 文件名 -> FileNode
    std::unordered_map<std::string, std::shared_ptr<FileNode>> children_;
    // std::vector<std::shared_ptr<FileNode>> children_;
};

// 文件树（表示需要打包的所有文件）
// 两种获取形式
// 1. 打包过程：根据文件路径，搜索所有需要打包的文件
// 2. 解包过程：根据打包文件，进行文件解析，构造tree
class FileTree
{
  public:
    FileTree()
        : root_(std::make_shared<FileNode>())
    {}
    ~FileTree() = default;

    /**
     * @brief 在文件树上添加文件，将磁盘路径为src的文件，打包到
     * 相对路径dest上
     * @param src 磁盘路径（文件/文件夹）（相对/绝对路径，内部会处理)
     * @param dest 打包路径（不含开始/，结尾不含/）
     * @param recursively 对于文件夹是否要递归处理
     */
    void
    PackFileAdd(const Path &src, const Path &dest, bool recursively = true);

    /**
     * @brief 全量备份，落盘到磁盘上
     * @param ofs 输出流
     */
    void FullDump(std::ofstream &ofs);

    /**
     * @brief 从输入流中构建FileTree
     * @param ifs
     */
    void Load(std::ifstream &ifs);

    /**
     * @brief 将打包文件中 pack_path 所指的 filenode 恢复到 target_path 中
     * @param pack_path 打包路径（不含开始的/，结尾的/）
     * @param ifs 打包文件的ifs（用于读取data，恢复regular文件）
     *        不要求文件流指针的位置
     * @param target_path 恢复到的目的路径，必须是文件夹，不存在则创建
     */
    void Recover(
        const std::string &pack_path,
        std::ifstream &ifs,
        const std::string &target_path);

    /**
     * @brief 返回tree的根节点
     */
    std::shared_ptr<FileNode> GetRootNode() { return root_; };

  private:
    void PackFileAdd(
        const Path &path,
        std::shared_ptr<FileNode> file_node,
        bool recursively = true);
    /**
     * @brief 保存被软连接指向的文件
     * @param path 相对/绝对路径（内部会转成绝对）
     * @param file_node 软链接文件的文件节点
     */
    void SaveBeLinked(const Path &path, std::shared_ptr<FileNode> file_node);
    void InitHeader();
    void InitMeta();
    void InitLinkMeta();
    void DumpMetaAndLinkMeta(std::ofstream &ofs);
    void DumpInitNode(std::shared_ptr<FileNode> root, uint64_t &data_offset);
    void DumpData(std::ofstream &ofs);
    void Recover(
        std::shared_ptr<FileNode> pack_node,
        std::ifstream &ifs,
        const std::string &target_path);
    void RecoverRegularFile(
        std::shared_ptr<FileNode> pack_node,
        std::ifstream &ifs,
        const std::string &target_path);

    /**
     * @brief 磁盘->tree的过程中，可能乱序，所以可能提前创建dir
     */
    std::shared_ptr<FileNode> LocateAndCreateDir(const Path &path);

    /**
     * @brief 根据打包路径，找到node的位置
     * @return node节点，如果找不到返回nullptr
     */
    std::shared_ptr<FileNode> LocateNode(const Path &path);
    BackupFileHeader header_;
    std::shared_ptr<FileNode> root_; // dummy node 虚拟根节点
    // 备份被链接的文件， <绝对路径，node>
    std::unordered_map<std::string, std::shared_ptr<FileNode>> file_be_linked_;
    // 打包->落盘时考虑
    // 解包->保存文件时也考虑
    // 最后的string是实际存储的路径：
    // - 如果为 打包->落盘，无含义
    // - 如果为
    // 解包->保存文件，已经dump下来的文件路径，如果为空，说明还没有被dump
    std::unordered_map<ino_t, std::pair<std::shared_ptr<FileNode>, std::string>>
        ino_map_; // 用于记录inode号->文件节点
    // 顺序记录要保存的node
    std::vector<std::shared_ptr<FileNode>> nodes_to_save_;
    size_t count_ = 0; // 文件 和 文件夹个数 (不包含软链接指向的文件)
};

} // namespace backup
