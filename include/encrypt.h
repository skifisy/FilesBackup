////
// @file crypto.h
// @brief
// 加密库
//
// @author yuhaoze
// @email 743544510@qq.com
//

#pragma once

#include <cryptopp/files.h>
#include <cryptopp/hex.h>
#include <cryptopp/sha.h>

#include <iostream>
using namespace CryptoPP;

namespace backup {

std::string calculate_sha256(const std::string& filename) {
  std::string digest;
  try {
    SHA256 hash;
    // 从文件中计算 SHA-256 哈希值
    FileSource file(
        filename.c_str(), true,
        new HashFilter(hash, new HexEncoder(new StringSink(digest))));

    // std::cout << "SHA-256 hash of " << filename << ": " << digest <<
    // std::endl;
  } catch (const Exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  return digest;
}
}  // namespace backup
