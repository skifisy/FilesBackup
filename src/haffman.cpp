////
// @file haffman.cpp
// @brief
// 哈夫曼树
//
// @author yuhaoze
// @email 743544510@qq.com
//
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
}

size_t Haffman::DumpFreq(std::ofstream& ofs) {
  size_t ret = 0;
  // 1. 写入dump的条数
  uint8_t count = charFreq.size();
  ret += DumpVar(count, ofs);
  // 2. 逐条写入
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
  // 1. 读入条数
  uint8_t count;
  ret += LoadVar(count, ifs);
  // 2. 逐条读入
  for (uint8_t i = 0; i < count; i++) {
    uint8_t ch;
    uint64_t freq;
    ret += LoadVar(ch, ifs);
    ret += LoadVar(freq, ifs);
    charFreq[ch] = freq;
  }
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
  // 左子树
  createHaffmanCodeSub(root->left, len + 1, code);
  code.set(len);
  // 右子树
  createHaffmanCodeSub(root->right, len + 1, code);
}

}  // namespace backup
