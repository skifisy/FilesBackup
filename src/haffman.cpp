////
// @file haffman.cpp
// @brief
// 哈夫曼树
//
// @author yuhaoze
// @email 743544510@qq.com
//
#include "haffman.h"
namespace backup {

void Haffman::deleteNode(Node* node) {
  if (node != nullptr) {
    deleteNode(node->left);
    deleteNode(node->right);
    delete node;
  } else {
    return;
  }
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
std::unordered_map<char, std::string> Haffman::createHaffmanCode() {
  Node* root = nodeQueue.top();
  if (root == nullptr) return codes;
  if (root->left == nullptr && root->right == nullptr) {
    codes[root->ch] = "0";
    return codes;
  }
  createHaffmanCodeSub(root, "");
  return codes;
}
void Haffman::createHaffmanCodeSub(Node* root, const std::string& code) {
  if (!root) return;

  if (root->left == nullptr && root->right == nullptr) {
    codes[root->ch] = code;
    return;
  }

  createHaffmanCodeSub(root->left, code + "0");
  createHaffmanCodeSub(root->right, code + "1");
}

}  // namespace backup
