#include <pack.h>

void DumpString(const std::string &str, ofstream &ofs) {
  uint32_t length = str.size();
  ofs.write(reinterpret_cast<const char *>(&length), sizeof(length));
  ofs.write(str.c_str(), length);
}

void DumpInt32(uint32_t num, ofstream &ofs) {
  ofs.write(reinterpret_cast<const char *>(&num), sizeof(num));
}

void DumpInt64(uint64_t num, ofstream &ofs) {
  ofs.write(reinterpret_cast<const char *>(&num), sizeof(num));
}

void LoadString(std::string &str, ifstream &ifs) {
  uint32_t length;
  ifs.read(reinterpret_cast<char *>(&length), sizeof(length));
  str.resize(length);
  ifs.read(&str[0], length);
}

void LoadInt32(uint32_t &num, ifstream &ifs) {
  ifs.read(reinterpret_cast<char *>(&num), sizeof(num));
}

void LoadInt64(uint64_t &num, ifstream &ifs) {
  ifs.read(reinterpret_cast<char *>(&num), sizeof(num));
}

void BackupFileHeader::Dump(ofstream &ofs)
{
}

void BackupFileHeader::Load(ifstream &ifs)
{
}
