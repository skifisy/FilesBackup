////
// @file haffman.cpp
// @brief
// 哈夫曼树
//
// @author yuhaoze
// @email 743544510@qq.com
//
#include <cassert>
#include <iostream>

#include "file_meta.h"
#include "haffman.h"
namespace backup {
constexpr int BUFFER_SIZE = 1024;
void Haffman::deleteNode(Node* node) {
  if (node != nullptr) {
    deleteNode(node->left);
    deleteNode(node->right);
    delete node;
  } else {
    return;
  }
}

void Haffman::CountFreq(std::ifstream& ifs) {
  char buffer[BUFFER_SIZE];
  while (!ifs.eof()) {
    int len = ifs.read(buffer, BUFFER_SIZE).gcount();
    for (int i = 0; i < len; i++) {
      charFreq[buffer[i]]++;
    }
  }
  for (auto [ch, freq] : charFreq) {
    nodeQueue.push(new Node(ch, freq));
  }
}

size_t Haffman::DumpFreq(std::ofstream& ofs) {
  size_t ret = 0;
  // 1. 写入编码后的bit数
  file_len_pos = ofs.tellp();
  ret += DumpVar(file_len, ofs);
  // 2. 写入dump的条数
  uint8_t count = charFreq.size();
  ret += DumpVar(count, ofs);
  // 3. 逐条写入
  for (auto& pair : charFreq) {
    uint8_t ch = pair.first;
    uint64_t freq = pair.second;
    ret += DumpVar(ch, ofs);
    ret += DumpVar(freq, ofs);
  }
  return ret;
}

size_t Haffman::RecoverFreq(std::ifstream& ifs) {
  size_t ret = 0;
  // 1. 读入编码后的bit数
  ret += LoadVar(file_len, ifs);
  // 2. 读入条数
  uint8_t count;
  ret += LoadVar(count, ifs);
  // 3. 逐条读入
  for (uint8_t i = 0; i < count; i++) {
    uint8_t ch;
    uint64_t freq;
    ret += LoadVar(ch, ifs);
    ret += LoadVar(freq, ifs);
    charFreq[ch] = freq;
    nodeQueue.push(new Node(ch, freq));
  }
  return ret;
}

size_t Haffman::CompressFile(std::ifstream& ifs, std::ofstream& ofs) {
  size_t ret = 0;
  // 将源文件按照字节进行编码，最后存储到压缩文件中
  // 按照256二进制位单位写入
  int remain = 256;          // 剩余可写的位数
  std::bitset<256> cur = 0;  // 当前需要写入的bitset
  // 缓冲区（一次读入多个字节）
  char buffer[BUFFER_SIZE];
  while (!ifs.eof()) {
    assert(ifs.good());
    ifs.read(buffer, BUFFER_SIZE);
    assert(!ifs.bad());
    size_t count = ifs.gcount();
    for (size_t i = 0; i < ifs.gcount(); i++) {
      char byte = buffer[i];
      int code_len = codes[byte].first;
      std::bitset<256> code = codes[byte].second;
      if (code_len <= remain) {
        // example: A: 110 pos: 256
        // cur = 110....
        cur |= code << (remain - code_len);
        remain -= code_len;
      } else {
        // example: A: 11110 remain: 2
        // cur = xx...11
        cur |= code >> (code_len - remain);
        DumpBitSet(cur, 256, ofs);
        ret += 256;
        cur = 0;
        code = codes[byte].second;
        cur |= code << (256 - remain);  // 256 减去之前用掉的位数
        // cur = 110...
        remain = 256 - (code_len - remain);
      }
    }
    if (remain != 256) {
      // 说明还剩了几位
      // return remain;  // 保存剩余的位数
      DumpBitSet(cur, 256 - remain, ofs);
      ret += (256 - remain);
    }
  }
  file_len = ret;
  // 回填file_len
  ofs.seekp(file_len_pos);
  DumpVar(file_len, ofs);
  ofs.seekp(0, std::ios::end);
  return ret;
}

size_t Haffman::UnCompressFile(std::ifstream& ifs, std::ofstream& ofs) {
  // 逐比特恢复一个char
  char buffer[BUFFER_SIZE];
  size_t idx = 0;
  assert(!nodeQueue.empty());
  Node* cur_node = nodeQueue.top();
  std::bitset<256> set = 0;
  size_t ret = 0;
  for (size_t i = 0; i < file_len / 256; i++) {
    LoadBitSet(set, 256, ifs);
    for (size_t pos = 255; pos >= 0; pos--) {
      if (set[pos]) {
        cur_node = cur_node->right;
      } else {
        cur_node = cur_node->left;
      }
      if (cur_node->left == nullptr && cur_node->right == nullptr) {
        buffer[idx++] = cur_node->ch;
        if (idx == BUFFER_SIZE) {
          ofs.write(buffer, BUFFER_SIZE);
          idx = 0;
          ret += BUFFER_SIZE;
        }
        cur_node = nodeQueue.top();
      }
    }
  }
  int l = file_len % 256;
  if (l != 0) {
    LoadBitSet(set, l, ifs);
    for (size_t pos = 255; pos > 255 - l; pos--) {
      if (set[pos]) {
        cur_node = cur_node->right;
      } else {
        cur_node = cur_node->left;
      }
      if (cur_node->left == nullptr && cur_node->right == nullptr) {
        buffer[idx++] = cur_node->ch;
        if (idx == BUFFER_SIZE) {
          ofs.write(buffer, BUFFER_SIZE);
          idx = 0;
          ret += BUFFER_SIZE;
        }
        cur_node = nodeQueue.top();
      }
    }
  }
  if (idx > 0) {
    ofs.write(buffer, idx);
    ret += idx;
  }
  assert(nodeQueue.top() == cur_node);
  return ret;
}

// 构建哈夫曼树
Node* Haffman::createHaffmanTree() {
  while (nodeQueue.size() > 1) {
    Node* left = nodeQueue.top();
    nodeQueue.pop();
    Node* right = nodeQueue.top();
    nodeQueue.pop();

    // '\0' 表示合并后的新节点
    Node* merged = new Node('\0', left->freq + right->freq);
    merged->left = left;
    merged->right = right;

    nodeQueue.push(merged);
  }

  return nodeQueue.top();
}

// 生成哈夫曼编码表
std::unordered_map<char, std::pair<int, std::bitset<256>>>
Haffman::createHaffmanCode() {
  Node* root = nodeQueue.top();
  if (root == nullptr) return codes;
  if (root->left == nullptr && root->right == nullptr) {
    // codes[root->ch] = "0";
    std::pair<int, std::bitset<256>> zero =
        std::make_pair(1, std::bitset<256>(0));
    codes[root->ch] = zero;
    return codes;
  }
  createHaffmanCodeSub(root, 0, std::bitset<256>(0));
  return codes;
}

// len为当前code的位数
void Haffman::createHaffmanCodeSub(Node* root, int len, std::bitset<256> code) {
  if (!root) return;

  if (root->left == nullptr && root->right == nullptr) {
    // codes[root->ch] = code;
    codes[root->ch] = std::make_pair(len, code);
    return;
  }
  code<<= 1;
  // 左子树
  createHaffmanCodeSub(root->left, len + 1, code);
  // code.set(len);
  // 右子树
  code.set(0);
  createHaffmanCodeSub(root->right, len + 1, code);
}

// 注意：必须从高位字节->低位字节编码
size_t Haffman::DumpBitSet(std::bitset<256> set, int len, std::ofstream& ofs) {
  size_t ret = len / 8 + (len % 8 ? 1 : 0);
  std::bitset<256> mask(uint8_t(-1));
  // 将len按照字节对齐写入
  for (size_t i = 0; i < len / 8; i++) {
    uint8_t byte = 0;
    // 取出最高的8位
    byte = static_cast<uint8_t>((set >> (256 - 8)).to_ulong());
    ofs << byte;
    set <<= 8;
  }
  if (len % 8 != 0) {
    uint8_t byte = 0;
    byte = static_cast<uint8_t>((set >> (256 - 8)).to_ulong());
    ofs << byte;
  }
  return ret;
}

size_t Haffman::LoadBitSet(std::bitset<256>& set, int len, std::ifstream& ifs) {
  set = 0;
  size_t ret = len / 8 + (len % 8 ? 1 : 0);
  for (size_t i = 0; i < len / 8; i++) {
    uint8_t byte = 0;
    ifs >> byte;
    std::bitset<256> bset(byte);
    set |= bset;
    set <<= 8;
  }
  // 最后剩下一个字节
  if (len % 8 != 0) {
    uint8_t byte = 0;
    ifs >> byte;
    std::bitset<256> bset(byte);
    set |= bset;
  }
  // 补齐到高位
  // b1 b2 b3 b4 b5
  if (ret < 256 / 8) {
    set <<= (256 / 8 - ret) * 8;
  }
  return ret;
}

}  // namespace backup
