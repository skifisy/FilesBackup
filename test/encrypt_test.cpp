#include <gtest/gtest.h>
#include <openssl/rand.h>

#include <cstring>
#include <fstream>
#include <iostream>
#define private public
#include "encrypt.h"

using namespace backup;

TEST(EncryptTest, EncryptFile) {
  ::system("echo \"hello world\" > test.txt ");

  // 加密和解密示例
  std::string input_file = "test.txt";
  std::string encrypted_file = "encrypted.dat";
  std::string decrypted_file = "decrypted.txt";
  std::ifstream ifs(input_file, std::ios::binary);
  std::ofstream ofs(encrypted_file, std::ios::binary);

  std::string password("abcabc");
  Encrypt enc(ifs, ofs);
  enc.AES_encrypt_file(password);
  ifs.close();
  ofs.close();

  std::ifstream ifss(encrypted_file, std::ios::binary);
  std::ofstream ofss(decrypted_file, std::ios::binary);
  Encrypt enc2(ifss, ofss);

  EXPECT_FALSE(enc2.AES_decrypt_file("hello"));
  ifss.clear();
  ifss.seekg(0);
  EXPECT_TRUE(enc2.AES_decrypt_file(password));
  ifss.close();
  ofss.close();

  EXPECT_EQ(memcmp(enc.salt, enc2.salt, 16), 0);
  EXPECT_EQ(memcmp(enc.iv, enc2.iv, 16), 0);
  EXPECT_EQ(memcmp(enc.key, enc2.key, 32), 0);

  EXPECT_EQ(::system("cmp test.txt decrypted.txt"), 0);
  ::system("rm -f test.txt encrypted.dat decrypted.txt");
}

TEST(EncryptTest, SHA256Test) {
  std::ofstream ofs("test.txt");
  std::string output("hello world");
  ofs.write(output.c_str(), output.size());
  ofs.flush();
  unsigned char hash[SHA256_SIZE];
  std::ifstream ifs("test.txt");
  EXPECT_TRUE(compute_file_sha256(ifs, hash));
  EXPECT_EQ(HashToHexString(hash, SHA256_SIZE),"b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9");
  ::system("rm -f test.txt");
}