#include <iostream>
#include <memory>
#include "backup_impl.h"

int main() {
  std::unique_ptr<BackUp> backup = std::make_unique<BackUpImpl>();
  backup->Copy("./test1", "./to");
}