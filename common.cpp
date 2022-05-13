#include "common.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CHECK_ERR(S, MSG) \
  if ((S) < 0) {          \
    perror(MSG);          \
    exit(1);              \
  }

int get_mount_file(std::string mount_path) {
  int fd;
  if (access(mount_path.c_str(), F_OK) != 0) {
    fd = creat(mount_path.c_str(), 0644);
    CHECK_ERR(fd, "creat");
    CHECK_ERR(pwrite(fd, "\x00", 1, 1024ll * 1024 * 1024 * 1024), "pwrite");
  } else {
    fd = open(mount_path.c_str(), O_RDWR);
    CHECK_ERR(fd, "open");
  }
  return fd;
}
