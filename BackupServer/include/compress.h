////
// @file compress.h
// @brief
// 文件压缩
//
// @author hz
// @email skyfishine@163.com
//
#pragma once
#include "haffman.h"
namespace backup {
class Compress
{
  public:
    void CompressFile();
    void DecompressFile();

    Compress(std::ifstream &src, std::ofstream &dest)
        : src(src)
        , dest(dest)
    {}

  private:
    std::ifstream &src;
    std::ofstream &dest;
    Haffman haff_tree;
};
} // namespace backup
