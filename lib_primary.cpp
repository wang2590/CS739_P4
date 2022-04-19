#include "lib_primary.h"

#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>

#include "common.h"

#define CHECK_ERR(S, MSG) \
  if ((S) < 0) {          \
    perror(MSG);          \
    return false;         \
  }

bool LibPrimary::Write(int offset, const std::string& buf) {
  assert(buf.size() == kBlockSize);

  int tmp_fd = creat(kTempFilesPath, 0644);
  CHECK_ERR(tmp_fd, "creat");
  CHECK_ERR(write(tmp_fd, reinterpret_cast<char*>(&offset), sizeof(offset)),
            "write");
  CHECK_ERR(write(tmp_fd, buf.data(), kBlockSize), "write");
  CHECK_ERR(close(tmp_fd), "close");

  CHECK_ERR(pwrite(file_fd_, buf.data(), kBlockSize, offset), "pwrite");
  CHECK_ERR(fsync(file_fd_), "fsync");

  if (sync_backup_) {
    int err = primary_backup_client_->clientSyncData({offset});
    if (err) sync_backup_ = false;
  }

  // Check `sync_backup_` again instead of using `else` because `sync_backup_`
  // may become `false` if it fails to sync.
  if (!sync_backup_) {
    restore_queue_lock_.lock();
    restore_queue_.push_back(offset);
    restore_queue_lock_.unlock();
  }

  CHECK_ERR(unlink(kTempFilesPath), "unlink");  // TODO: Recovery with temp file

  return true;
}

bool LibPrimary::Read(int offset, std::string& buf) {
  buf.resize(kBlockSize);
  int res = pread(file_fd_, buf.data(), kBlockSize, offset);
  CHECK_ERR(res, "pread");
  return res == kBlockSize;
}

void LibPrimary::BackupAlive() {
  std::lock_guard<std::mutex> guard(restore_queue_lock_);

  if (!sync_backup_) {
    std::sort(restore_queue_.begin(), restore_queue_.end());
    restore_queue_.resize(
        std::unique(restore_queue_.begin(), restore_queue_.end()) -
        restore_queue_.begin());

    int res = primary_backup_client_->clientSyncData(restore_queue_);

    if (res == 0) {
      restore_queue_.clear();
      sync_backup_ = true;
    }
  }
}
