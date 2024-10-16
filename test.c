#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void printTime(time_t t) {
  struct tm *ts = localtime(&t);
  printf("%d-%d-%d %02d:%02d", ts->tm_year + 1900, ts->tm_mon + 1, ts->tm_mday, ts->tm_hour, ts->tm_min);
}

void printFileType(mode_t mode) {
  switch (mode & S_IFMT) {
    case S_IFREG:  // 普通文件
      printf("a regular file.\n");
      break;
    case S_IFDIR:  // 目录文件
      printf("a directory.\n");
      break;
    case S_IFSOCK:  // 套接字文件
      printf("a socket.\n");
      break;
    case S_IFCHR:  // 字符设备
      printf("a character device.\n");
      break;
    case S_IFIFO:  // 管道
      printf("a FIFO.\n");
      break;
    case S_IFBLK:  // 块文件
      printf("a block device.\n");
      break;
    case S_IFLNK:
      printf("a symbolic link.\n");
      break;
    default:
      printf("an unknown type.\n");
      break;
  }
}

int main() {
  DIR *dirp = opendir(".");
  struct dirent *dp;
  long loc;

  // 读取并打印前两个目录项
  dp = readdir(dirp);
  printf("1: %s\n", dp->d_name);
  dp = readdir(dirp);
  printf("2: %s\n", dp->d_name);

  // 获取当前位置
  loc = telldir(dirp);

  // 重置位置指针
  rewinddir(dirp);

  // 跳转到之前保存的位置
  seekdir(dirp, loc);

  // 读取并打印下一个目录项
  dp = readdir(dirp);
  printf("3: %s\n", dp->d_name);

  // 打印所有目录项：
  rewinddir(dirp);
  struct dirent *entry;
  int cur = 0;
  struct stat st;
  while ((entry = readdir(dirp)) != NULL) {
    printf("%d: %s\n", cur++, entry->d_name);
    if (lstat(entry->d_name, &st) == 0) {
      mode_t mode = st.st_mode;
      time_t atime = st.st_atime;
      time_t mtime = st.st_mtime;
      printf("\t%o ", mode);
      printf("%d %d %d ", st.st_ino, st.st_dev, st.st_nlink); // 索引节点号 设备号 硬链接数
      printTime(atime);
      printf(" ");
      printTime(mtime);
      printf("\n\t");
      printFileType(mode);
    }
  }

  closedir(dirp);
  return 0;
}
