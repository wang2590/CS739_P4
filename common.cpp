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

int get_mount_file() {
  int fd;
  if (access(kMountPath, F_OK) != 0) {
    fd = creat(kMountPath, 0644);
    CHECK_ERR(fd, "creat");
    CHECK_ERR(pwrite(fd, "\x00", 1, 1024ll * 1024 * 1024 * 1024), "pwrite");
  } else {
    fd = open(kMountPath, O_RDWR);
    CHECK_ERR(fd, "open");
  }
  return fd;
}
