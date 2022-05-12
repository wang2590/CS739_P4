#include <string>

constexpr int kBlockSize = 4 * 1024;
constexpr char kTempFilesPath[] = "./write_temp_file";

int get_mount_file(std::string mount_path);
