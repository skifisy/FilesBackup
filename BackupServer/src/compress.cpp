////
// @file compress.cpp
// @brief
// 文件压缩 使用哈夫曼编码
//
// @author yuhaoze
// @email 743544510@qq.com
//
#include "compress.h"

namespace backup {
void Compress::CompressFile()
{
    haff_tree.CountFreq(src);
    haff_tree.createHaffmanTree();
    haff_tree.createHaffmanCode();
    haff_tree.DumpFreq(dest);
    src.clear();
    src.seekg(0);
    haff_tree.CompressFile(src, dest);
}

void Compress::DecompressFile()
{
    haff_tree.RecoverFreq(src);
    haff_tree.createHaffmanTree();
    haff_tree.createHaffmanCode();
    haff_tree.UnCompressFile(src, dest);
}

} // namespace backup
