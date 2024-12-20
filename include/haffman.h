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
  // 获取哈夫曼编码
  std::unordered_map<char, std::string> createHaffmanCode();
  // 生成一个哈夫曼树
  Node *createHaffmanTree();

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
  void createHaffmanCodeSub(Node *root, const std::string &code);
  // 用于生成哈夫曼树的优先队列
  std::priority_queue<Node *, std::vector<Node *>, Compare> nodeQueue;
  // 哈夫曼编码表
  std::unordered_map<char, std::string> codes;
};

}  // namespace backup
