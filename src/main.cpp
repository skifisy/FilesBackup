#include <iostream>
#include <memory>

#include "backup_impl.h"
#include "encrypt.h"
#include "file_sys.h"

using namespace backup;

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
    return 1;
  }
  std::string p = argv[1];
  auto files = GetFilesFromDir(p);

  return 0;
}