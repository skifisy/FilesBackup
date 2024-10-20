////
// @file pack.h
// @brief
// 文件打包
//
// @author yuhaoze
// @email 743544510@qq.com
//
#pragma once
class FilePack {
 public:
  virtual void Pack() = 0;
  virtual void UnPack() = 0;
  virtual void PackBatch() = 0;
  virtual void UnPackBatch() = 0;
  FilePack() = default;
  ~FilePack() = default;
};