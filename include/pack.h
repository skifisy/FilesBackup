////
// @file pack.h
// @brief
// 文件打包
//
// @author yuhaoze
// @email 743544510@qq.com
//
#pragma once
#include <string>
namespace backup {

class FilePack {
 public:
  virtual void Pack(std::string src, std::string dest) = 0;
  virtual void UnPack(std::string src, std::string dest) = 0;
  virtual void PackBatch() = 0;
  virtual void UnPackBatch() = 0;
  FilePack() = default;
  ~FilePack() = default;
};
}  // namespace backup
