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

std::string calculate_sha256(const std::string& filepath);
}  // namespace backup
