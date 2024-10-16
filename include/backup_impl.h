#pragma once
#include "backup.h"

class BackUpImpl : public BackUp {
 public:
  BackUpImpl() = default;
  ~BackUpImpl() = default;
  // 将src中的所有文件保存到dest中
  void Copy(std::string src, std::string dest);
  // 将目录树中的文件保存到指定位置
  // TODO: 压缩、打包、加密
  void Save(std::string src, std::string dest);
  // 将目录树中的文件恢复到指定位置
  // TODO: 解压、解包、解密
  void Recover(std::string src, std::string dest);

 private:
  /* data */
};