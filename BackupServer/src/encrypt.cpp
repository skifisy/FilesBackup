////
// @file encrypt.cpp
// @brief
// 加密
//
// @author hz
// @email skyfishine@163.com
//
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <cstring>
#include <fstream>
#include <iomanip> // for std::hex and std::setw
#include <iostream>
#include <string>

#include "encrypt.h"
namespace backup {

// 使用PBKDF2派生256位AES密钥
/**
 * @param password 用户输入的密码
 * @param salt 16字节的盐值
 * @param aes_key 输出的256位的AES密钥
 */
void derive_key(
    const std::string &password,
    unsigned char *salt,
    unsigned char *aes_key)
{
    // PBKDF2-HMAC-SHA256密钥派生
    // 参数：password: 用户输入的密码，salt: 盐值（可以是随机生成的），aes_key:
    // 目标密钥（256位）
    const int iterations = RETRY_COUNT; // 迭代次数
    if (!PKCS5_PBKDF2_HMAC(
            password.c_str(),
            password.length(),
            salt,
            AES_BLOCK_SIZE,
            iterations,
            EVP_sha256(),
            32,
            aes_key)) {
        std::cerr << "PBKDF2 key derivation failed." << std::endl;
        exit(1);
    }
}

void Encrypt::AES_encrypt_file(const std::string &password)
{
    // 生成初始化向量（IV）
    if (!RAND_bytes(iv, sizeof(iv))) {
        std::cerr << "Random IV generation failed." << std::endl;
        return;
    }
    if (!RAND_bytes(salt, sizeof(salt))) {
        std::cerr << "Random bytes generation failed." << std::endl;
        return;
    }
    // 向加密文件中写入salt
    output_file.write(reinterpret_cast<char *>(salt), sizeof(salt));
    // 向加密文件中写入iv
    output_file.write(reinterpret_cast<char *>(iv), sizeof(iv));

    derive_key(password, salt, key);

    // 初始化EVP上下文
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "初始化EVP失败" << std::endl;
        return;
    }

    // 初始化加密操作（AES-256-CBC）
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        std::cerr << "初始化加密失败" << std::endl;
        return;
    }

    unsigned char in_buffer[1024], out_buffer[1024 + EVP_MAX_BLOCK_LENGTH];
    int in_len, out_len;

    while (input_file.read(
        reinterpret_cast<char *>(in_buffer), sizeof(in_buffer))) {
        in_len = input_file.gcount();
        if (1 !=
            EVP_EncryptUpdate(ctx, out_buffer, &out_len, in_buffer, in_len)) {
            std::cerr << "加密失败" << std::endl;
            EVP_CIPHER_CTX_free(ctx);

            return;
        }
        output_file.write(reinterpret_cast<char *>(out_buffer), out_len);
    }

    // 处理剩余数据
    if (input_file.gcount() > 0) {
        in_len = input_file.gcount();
        if (1 !=
            EVP_EncryptUpdate(ctx, out_buffer, &out_len, in_buffer, in_len)) {
            std::cerr << "加密失败" << std::endl;
            EVP_CIPHER_CTX_free(ctx);

            return;
        }
        output_file.write(reinterpret_cast<char *>(out_buffer), out_len);
    }

    // 完成加密操作
    if (1 != EVP_EncryptFinal_ex(ctx, out_buffer, &out_len)) {
        std::cerr << "加密失败" << std::endl;
        EVP_CIPHER_CTX_free(ctx);

        return;
    }
    output_file.write(reinterpret_cast<char *>(out_buffer), out_len);

    EVP_CIPHER_CTX_free(ctx);
}

bool Encrypt::AES_decrypt_file(const std::string &password)
{
    input_file.read(reinterpret_cast<char *>(salt), sizeof(salt));
    input_file.read(reinterpret_cast<char *>(iv), sizeof(iv));
    derive_key(password, salt, key);

    // 初始化EVP上下文
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "初始化EVP失败" << std::endl;
        return false;
    }

    // 初始化解密操作（AES-256-CBC）
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)) {
        std::cerr << "初始化解密失败" << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    unsigned char in_buffer[1024], out_buffer[1024 + EVP_MAX_BLOCK_LENGTH];
    int in_len, out_len;

    while (input_file.read(
        reinterpret_cast<char *>(in_buffer), sizeof(in_buffer))) {
        in_len = input_file.gcount();
        if (1 !=
            EVP_DecryptUpdate(ctx, out_buffer, &out_len, in_buffer, in_len)) {
            std::cerr << "EVP_DecryptUpdate fail" << std::endl;
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        output_file.write(reinterpret_cast<char *>(out_buffer), out_len);
    }

    // 处理剩余数据
    if (input_file.gcount() > 0) {
        in_len = input_file.gcount();
        if (1 !=
            EVP_DecryptUpdate(ctx, out_buffer, &out_len, in_buffer, in_len)) {
            std::cerr << "EVP_DecryptUpdate fail" << std::endl;
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        output_file.write(reinterpret_cast<char *>(out_buffer), out_len);
    }

    // 完成解密操作
    if (1 != EVP_DecryptFinal_ex(ctx, out_buffer, &out_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    output_file.write(reinterpret_cast<char *>(out_buffer), out_len);

    EVP_CIPHER_CTX_free(ctx);
    return true;
}

bool compute_file_sha256(std::ifstream &file, unsigned char *hash)
{
    constexpr size_t buffer_size = 4096; // Buffer size for file reading
    unsigned char buffer[buffer_size];
    unsigned int hash_len = 0;
    const EVP_MD *hash_function = EVP_sha256();

    // Initialize OpenSSL context
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    if (!md_ctx) {
        std::cerr << "Failed to create EVP_MD_CTX" << std::endl;
        return false;
    }

    if (EVP_DigestInit_ex(md_ctx, hash_function, nullptr) != 1) {
        std::cerr << "EVP_DigestInit_ex failed" << std::endl;
        EVP_MD_CTX_free(md_ctx);
        return false;
    }

    // Read file and update hash
    while (file.good()) {
        file.read(reinterpret_cast<char *>(buffer), buffer_size);
        size_t bytes_read = file.gcount();
        if (bytes_read > 0) {
            if (EVP_DigestUpdate(md_ctx, buffer, bytes_read) != 1) {
                std::cerr << "EVP_DigestUpdate failed" << std::endl;
                EVP_MD_CTX_free(md_ctx);
                return false;
            }
        }
    }

    // Finalize the hash
    if (EVP_DigestFinal_ex(md_ctx, hash, &hash_len) != 1) {
        std::cerr << "EVP_DigestFinal_ex failed" << std::endl;
        EVP_MD_CTX_free(md_ctx);
        return false;
    }

    EVP_MD_CTX_free(md_ctx);

    return true;
}

std::string HashToHexString(const unsigned char *hash, size_t hash_len)
{
    // Convert hash to hexadecimal string
    std::ostringstream oss;
    for (unsigned int i = 0; i < hash_len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(hash[i]);
    }
    return oss.str();
}

} // namespace backup
