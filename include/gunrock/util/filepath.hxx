#pragma once

#include <string>

namespace gunrock {
namespace util {

/// @brief 提取文件名
/// @param path 文件路径
/// @param delim 分隔符
/// @return 文件名
std::string extract_filename(std::string path, std::string delim = "/") {
  size_t lastSlashIndex = path.find_last_of("/");
  return path.substr(lastSlashIndex + 1);
}

/// @brief 提取数据集名称(去除拓展名)
/// @param filename 文件名
/// @return 数据集名称
std::string extract_dataset(std::string filename) {
  size_t lastindex = filename.find_last_of(".");
  return filename.substr(0, lastindex);
}

/// @brief 判断图文件是否为矩阵市场交换格式(.mtx或.mmio文件)
/// @param filename 文件路径
bool is_market(std::string filename) {
  return ((filename.substr(filename.size() - 4) == ".mtx") ||
          (filename.substr(filename.size() - 5) == ".mmio"));
}

bool is_binary_csr(std::string filename) {
  return filename.substr(filename.size() - 4) == ".csr";
}

}  // namespace util
}  // namespace gunrock