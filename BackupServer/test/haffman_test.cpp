////
// @file haffman_test.cpp
// @brief
// 测试哈夫曼树
//
// @author yuhaoze
// @email 743544510@qq.com
//
#include <gtest/gtest.h>
#define private public
#include "haffman.h"
#include "compress.h"
using namespace backup;

TEST(HaffmanTest, CreateHaffmanCode)
{
    std::unordered_map<char, uint64_t> mp;
    mp['a'] = 3;
    Haffman haff(mp);
    [[maybe_unused]] auto tree = haff.createHaffmanTree();
    auto codes = haff.createHaffmanCode();
    EXPECT_EQ(codes['a'].second, std::bitset<256>(std::string("0")));

    mp['b'] = 1;
    mp['c'] = 2;
    mp['d'] = 1;
    Haffman haff2(mp);
    tree = haff2.createHaffmanTree();
    codes = haff2.createHaffmanCode();
    EXPECT_EQ(codes['a'].first, 1);
    EXPECT_EQ(codes['b'].first, 3);
    EXPECT_EQ(codes['c'].first, 2);
    EXPECT_EQ(codes['d'].first, 3);
}

TEST(HaffmanTest, DumpBitSet)
{
    Haffman haff;
    std::ofstream ofs("bitset", std::ios::binary);
    std::bitset<256> set1("11010111");
    set1.set(255);
    set1.set(253);
    set1.set(252);
    set1.set(225);
    set1.set(145);
    std::bitset<256> set2("11000011");
    set2.set(233);
    set2.set(253);
    set2.set(243);
    set2.set(231);
    std::bitset<256> set3 = 0;
    set3.set(255);
    set3.set(252);
    std::bitset<256> set4 = 0;
    set4.set(255);
    set4.set(250);
    set4.set(246);
    set4.set(247);
    haff.DumpBitSet(set1, 256, ofs);
    haff.DumpBitSet(set2, 256, ofs);
    haff.DumpBitSet(set3, 8, ofs);
    haff.DumpBitSet(set1, 256, ofs);
    haff.DumpBitSet(set4, 9, ofs);
    ofs.close();
    std::ifstream ifs("bitset", std::ios::binary);
    std::bitset<256> set;
    EXPECT_EQ(haff.LoadBitSet(set, 256, ifs), 32);
    EXPECT_EQ(set, set1);
    EXPECT_EQ(ifs.tellg(), 32);
    EXPECT_EQ(haff.LoadBitSet(set, 256, ifs), 32);

    EXPECT_EQ(set, set2);
    EXPECT_EQ(haff.LoadBitSet(set, 8, ifs), 1);
    EXPECT_EQ(set, set3);
    EXPECT_EQ(haff.LoadBitSet(set, 256, ifs), 32);
    EXPECT_EQ(set, set1);
    EXPECT_EQ(haff.LoadBitSet(set, 9, ifs), 2);
    EXPECT_EQ(set, set4);
    EXPECT_EQ(::system("rm -f bitset"), 0);
}

TEST(HaffmanTest, CompressFileTest)
{
    EXPECT_EQ(::system("echo \"abcdabcd\" > haff"), 0);
    Haffman haff;
    auto &codes = haff.codes;
    codes['a'] = std::make_pair(3, std::bitset<256>("110"));
    codes['b'] = std::make_pair(1, std::bitset<256>("0"));
    codes['c'] = std::make_pair(3, std::bitset<256>("101"));
    codes['d'] = std::make_pair(3, std::bitset<256>("100"));
    codes[0x0a] = std::make_pair(3, std::bitset<256>("111")); // \n

    Node *n0 = new Node(0xa, 0, nullptr, nullptr);
    Node *na = new Node('a', 0, nullptr, nullptr);
    Node *nb = new Node('b', 0, nullptr, nullptr);
    Node *nc = new Node('c', 0, nullptr, nullptr);
    Node *nd = new Node('d', 0, nullptr, nullptr);
    Node *n1 = new Node('\0', 0, na, n0);
    Node *n2 = new Node('\0', 0, nd, nc);
    Node *n3 = new Node('\0', 0, n2, n1);
    Node *root = new Node('\0', 0, nb, n3);
    haff.nodeQueue.push(root);
    std::ifstream ifs("haff", std::ios::binary);
    std::ofstream ofs("haff_t", std::ios::binary);
    ofs.write(
        reinterpret_cast<const char *>(&haff.file_len), sizeof(haff.file_len));
    ofs.flush();
    size_t size = haff.CompressFile(ifs, ofs);
    EXPECT_EQ(size, 23);
    // 编码后为：
    // 110 0 101 100 110 0 101 100 111
    // 1100 1011 0011 0010 1100 1110 最后补0
    // 0xcb32ce
    ofs.close();
    std::ifstream ifss("haff_t", std::ios::binary);
    std::bitset<256> set, answer(0xcb32ce);
    answer <<= (256 / 8 - 3) * 8;
    ifss.seekg(sizeof(haff.file_len));
    size = haff.LoadBitSet(set, size, ifss);
    EXPECT_EQ(set, answer);
    EXPECT_EQ(size, 3);

    ifss.seekg(sizeof(haff.file_len));
    std::ofstream ofss("haff_tt", std::ios::binary);
    size = haff.UnCompressFile(ifss, ofss);
    EXPECT_EQ(size, 9);
    ofss.close();
    EXPECT_EQ(::system("cmp haff haff_tt"), 0);
    EXPECT_EQ(::system("rm -f haff haff_t haff_tt"), 0);
}

TEST(HaffmanTest, DumpAndRecoverTest)
{
    EXPECT_EQ(::system("echo \"hello world\nnvvvvvvdddeeww\" > haff_d"), 0);
    std::ifstream ifs("haff_d", std::ios::binary);
    std::ofstream ofs("haff_dd", std::ios::binary);
    Haffman haff;
    haff.CountFreq(ifs);
    haff.createHaffmanTree();
    haff.createHaffmanCode();
    haff.DumpFreq(ofs);
    EXPECT_EQ(haff.file_len_pos, 0);
    ofs.flush();
    ifs.clear();
    ifs.seekg(0, std::ios::beg); // 注意：重新返回到开始的位置！！
    haff.CompressFile(ifs, ofs);
    ofs.close();
    std::ifstream ifss("haff_dd", std::ios::binary);
    std::ofstream ofss("haff_ddd", std::ios::binary);
    Haffman haff2;
    haff2.RecoverFreq(ifss);
    haff2.createHaffmanTree();
    haff2.createHaffmanCode();
    for (auto &[ch, freq] : haff.charFreq) {
        EXPECT_EQ(haff2.charFreq[ch], freq);
    }
    EXPECT_EQ(haff.file_len, haff2.file_len);
    for (auto &[ch, pair] : haff.codes) {
        auto &len = pair.first;
        auto &code = pair.second;
        EXPECT_EQ(haff2.codes[ch].first, len);
        EXPECT_EQ(haff2.codes[ch].second, code);
    }
    haff2.UnCompressFile(ifss, ofss);
    ofss.close();
    EXPECT_EQ(::system("cmp haff_d haff_ddd"), 0);
    EXPECT_EQ(::system("rm -f haff_d haff_dd haff_ddd"), 0);
}
TEST(HaffmanTest, DumpAndRecoverTest1)
{
    // 测试超过1024字节的文件
    EXPECT_EQ(::system("truncate -s 1028 testfile"), 0);
    std::ifstream ifs("testfile", std::ios::binary);
    std::ofstream ofs("testfile_compress", std::ios::binary);
    // 压缩：
    Haffman haff;
    haff.CountFreq(ifs);
    haff.createHaffmanTree();
    haff.createHaffmanCode();
    haff.DumpFreq(ofs);
    EXPECT_EQ(haff.file_len_pos, 0);
    ofs.flush();
    ifs.clear();
    ifs.seekg(0, std::ios::beg); // 注意：重新返回到开始的位置！！
    size_t size = haff.CompressFile(ifs, ofs);
    EXPECT_EQ(size, 1028);
    ofs.close();
    // 解压：
    Haffman haff2;
    std::ifstream ifss("testfile_compress", std::ios::binary);
    std::ofstream ofss("testfile_unpress", std::ios::binary);
    haff2.RecoverFreq(ifss);
    haff2.createHaffmanTree();
    haff2.createHaffmanCode();
    for (auto &[ch, freq] : haff.charFreq) {
        EXPECT_EQ(haff2.charFreq[ch], freq);
    }
    EXPECT_EQ(haff.file_len, haff2.file_len);
    for (auto &[ch, pair] : haff.codes) {
        auto &len = pair.first;
        auto &code = pair.second;
        EXPECT_EQ(haff2.codes[ch].first, len);
        EXPECT_EQ(haff2.codes[ch].second, code);
    }
    haff2.UnCompressFile(ifss, ofss);
    ofss.close();
    EXPECT_EQ(::system("cmp testfile testfile_unpress"), 0);
    EXPECT_EQ(::system("rm -f testfile testfile_compress testfile_unpress"), 0);
}

TEST(HaffmanTest, DumpAndRecoverTest2)
{
    std::ifstream ifs("test.jpg", std::ios::binary);
    std::ofstream ofs("test_compress.jpg", std::ios::binary);
    // 压缩：
    Haffman haff;
    haff.CountFreq(ifs);
    haff.createHaffmanTree();
    haff.createHaffmanCode();
    size_t freq_dump_size = haff.DumpFreq(ofs);

    EXPECT_EQ(haff.file_len_pos, 0);
    ofs.flush();
    ifs.clear();
    ifs.seekg(0, std::ios::beg); // 注意：重新返回到开始的位置！！
    haff.CompressFile(ifs, ofs);
    ofs.close();
    // 解压：
    Haffman haff2;
    std::ifstream ifss("test_compress.jpg", std::ios::binary);
    std::ofstream ofss("test_unpress.jpg", std::ios::binary);
    size_t recover_size = haff2.RecoverFreq(ifss);
    EXPECT_EQ(recover_size, freq_dump_size);
    EXPECT_EQ(ifss.tellg(), freq_dump_size);
    haff2.createHaffmanTree();
    haff2.createHaffmanCode();
    EXPECT_EQ(haff.charFreq.size(), haff2.charFreq.size());
    for (auto &[ch, freq] : haff.charFreq) {
        EXPECT_EQ(haff2.charFreq[ch], freq);
    }
    EXPECT_EQ(haff.file_len, haff2.file_len);
    for (auto &[ch, pair] : haff.codes) {
        auto &len = pair.first;
        auto &code = pair.second;
        EXPECT_EQ(haff2.codes[ch].first, len);
        EXPECT_EQ(haff2.codes[ch].second, code);
    }
    haff2.UnCompressFile(ifss, ofss);
    ifss.clear();
    ofss.close();
    EXPECT_EQ(::system("cmp test.jpg test_unpress.jpg"), 0);
    EXPECT_EQ(::system("rm -f test_compress.jpg test_unpress.jpg"), 0);
}

TEST(CompressTest, CompressAndDecompress)
{
    std::ifstream ifs("test.jpg", std::ios::binary);
    std::ofstream ofs("test_compress.jpg", std::ios::binary);
    // 压缩
    Compress comp1(ifs, ofs);
    comp1.CompressFile();
    ofs.flush();
    std::ifstream ifss("test_compress.jpg", std::ios::binary);
    std::ofstream ofss("test_unpress.jpg", std::ios::binary);
    // 解压
    Compress comp2(ifss, ofss);
    comp2.DecompressFile();
    ofss.flush();
    EXPECT_EQ(::system("cmp test.jpg test_unpress.jpg"), 0);
    EXPECT_EQ(::system("rm -f test_compress.jpg test_unpress.jpg"), 0);
}