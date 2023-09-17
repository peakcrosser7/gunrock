/**
 * @file build.hxx
 * @author Muhammad Osama (mosama@ucdavis.edu)
 * @brief
 * @version 0.1
 * @date 2020-10-07
 *
 * @copyright Copyright (c) 2020
 *
 */

#pragma once

#include <gunrock/graph/detail/build.hxx>
#include <gunrock/formats/formats.hxx>

namespace gunrock {
namespace graph {
namespace build {

/// @brief 由CSR格式矩阵构建图
/// @tparam space 内存类型
/// @tparam build_views 图视图
/// @param r 矩阵行数
/// @param c 矩阵列数
/// @param nnz 矩阵非零元数
/// @param Ap 行偏移数组
/// @param J 列索引数组
/// @param X 值数组
/// @param I 行索引数组
/// @param Aj 列偏移数组
/// @return 构造的图结构
template <memory_space_t space,
          view_t build_views,
          typename edge_t,
          typename vertex_t,
          typename weight_t>
auto from_csr(vertex_t const& r,
              vertex_t const& c,
              edge_t const& nnz,
              edge_t* Ap,
              vertex_t* J,
              weight_t* X,
              vertex_t* I = nullptr,
              edge_t* Aj = nullptr) {
  // static_assert(); // TODO: check for flags and nullptrs
  return detail::from_csr<space, build_views>(r, c, nnz, Ap, J, X, I, Aj);
}

template <memory_space_t space,
          view_t build_views,
          typename edge_t,
          typename vertex_t,
          typename weight_t>
auto from_csr(format::csr_t<space, vertex_t, edge_t, weight_t>& csr) {
  return from_csr<space, build_views>(
      csr.number_of_rows,               // rows
      csr.number_of_columns,            // columns
      csr.number_of_nonzeros,           // nonzeros
      csr.row_offsets.data().get(),     // row_offsets
      csr.column_indices.data().get(),  // column_indices
      csr.nonzero_values.data().get()   // values
  );
}

}  // namespace build
}  // namespace graph
}  // namespace gunrock