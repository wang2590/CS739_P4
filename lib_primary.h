#ifndef LIB_PRIMARY_H
#define LIB_PRIMARY_H

#include <functional>
#include <mutex>
#include <string>
#include <vector>

#include "primary_backup_gRPC/primary_backup_grpc_client.h"

// This class is used by the primary to communicate with the clients. When the
// primary fails and the backup is responsible for interacting with clients, the
// backup also uses this library.
class LibPrimary {
 public:
  LibPrimary(bool sync_backup, int file_fd,
             PrimaryBackupgRPCClient* primary_backup_client)
      : sync_backup_(sync_backup),
        file_fd_(file_fd),
        primary_backup_client_(primary_backup_client) {}

  bool Write(int offset, const std::string& buf);
  bool Read(int offset, std::string& buf);

  void BackupAlive();

  std::vector<int> GetRestoreQueue() {
    std::lock_guard<std::mutex> guard(restore_queue_lock_);
    return restore_queue_;
  }

 private:
  // This variable determines if it is needed to sync the written data to
  // backup.
  // When the primary uses this library, `sync_backup_` is `true` iff the
  // backup is alive. When the backup uses this library, `sync_backup_` is
  // always `false` because the backup only replace the primary's job when
  // the primary is dead.
  bool sync_backup_;

  // The fd of the data file.
  int file_fd_;

  PrimaryBackupgRPCClient* primary_backup_client_;

  // The offsets of blocks that are written when the other machine is dead.
  std::vector<int> restore_queue_;
  std::mutex restore_queue_lock_;
};

#endif
