////
// @file encrypt.cpp
// @brief
// 加密
//
// @author yuhaoze
// @email 743544510@qq.com
//
#include "encrypt.h"

namespace backup {
std::string calculate_sha256(const std::string& filepath) {
  std::string digest;
  try {
    SHA256 hash;
    // 从文件中计算 SHA-256 哈希值
    FileSource file(
        filepath.c_str(), true,
        new HashFilter(hash, new HexEncoder(new StringSink(digest))));

    // std::cout << "SHA-256 hash of " << filename << ": " << digest <<
    // std::endl;
  } catch (const Exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return digest;
}

}  // namespace backup
