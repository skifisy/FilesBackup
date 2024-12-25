////
// @file crypto.h
// @brief
// 加密库
//
// @author yuhaoze
// @email 743544510@qq.com
//

#pragma once
#include <iostream>

namespace backup {
#define RETRY_COUNT 10000
class Encrypt {
 public:
  // 加密文件
  void AES_encrypt_file(const std::string& password);
  // 解密文件
  bool AES_decrypt_file(const std::string& password);
  Encrypt(std::ifstream& ifs, std::ofstream& ofs)
      : input_file(ifs), output_file(ofs) {}

 private:
  std::ifstream& input_file;
  std::ofstream& output_file;
  unsigned char salt[16];
  unsigned char iv[16];
  unsigned char key[32];  // 256位密钥
};
}  // namespace backup
