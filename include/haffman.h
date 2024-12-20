////
// @file haffman.h
// @brief
// 哈夫曼树
//
// @author yuhaoze
// @email 743544510@qq.com
//
#pragma once
#include <ctype.h>

#include <fstream>
#include <bitset>
#include <queue>
#include <string>
#include <unordered_map>
namespace backup {
//每一个带权的结点
class Node {
 public:
  char ch;
  uint64_t freq;
  Node *left;
  Node *right;
  Node(char c, uint64_t freq) {
    this->ch = c;
    this->freq = freq;
    left = nullptr;
    right = nullptr;
  }
  Node(char c, uint64_t freq, Node *leftChild, Node *rightChild) {
    this->ch = c;
    this->freq = freq;
    left = leftChild;
    right = rightChild;
  }
};
// 比较器用于优先队列
struct Compare {
  bool operator()(Node *a, Node *b) { return a->freq > b->freq; }
};
class Haffman {
 public:
  // 输入需要压缩的文件，统计频次信息
  void CountFreq(std::ifstream &ifs);
  // 往文件中写入频次信息
  size_t DumpFreq(std::ofstream &ofs);
  // 输入需要解压的文件，恢复频次信息
  size_t RecoverFreq(std::ifstream &ifs);

  // 生成一个哈夫曼树
  Node *createHaffmanTree();
  // 获取哈夫曼编码
  std::unordered_map<char, std::pair<int, std::bitset<256>>>
  createHaffmanCode();
  Haffman() {}
  //构造方法
  Haffman(const std::unordered_map<char, uint64_t> &charFreq) {
    //通过map来构建一个以频率为权值的优先队列
    for (auto [ch, freq] : charFreq) {
      nodeQueue.push(new Node(ch, freq));
    }
  }

  ~Haffman() {
    Node *node = nodeQueue.top();
    deleteNode(node);
  }

 private:
  // 递归删除结点
  void deleteNode(Node *node);
  void createHaffmanCodeSub(Node *root, int len, std::bitset<256> code);
  // 统计频次信息
  std::unordered_map<char, uint64_t> charFreq;
  // 用于生成哈夫曼树的优先队列
  std::priority_queue<Node *, std::vector<Node *>, Compare> nodeQueue;
  // 哈夫曼编码表
  // first: 被编码的字符
  // second: 编码长度；编码
  //
  // 使用bitset：
  // 1. 方便之后计算
  // 2. 2^8种字符，最长的haffman路径为256-1
  std::unordered_map<char, std::pair<int, std::bitset<256>>> codes;
};

}  // namespace backup
