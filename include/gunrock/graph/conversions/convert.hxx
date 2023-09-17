/**
 * @file convert.hxx
 * @author Muhammad Osama (mosama@ucdavis.edu)
 * @brief
 * @version 0.1
 * @date 2020-12-04
 *
 * @copyright Copyright (c) 2020
 *
 */

#include <gunrock/cuda/cuda.hxx>

namespace gunrock {
namespace graph {
namespace convert {

/// @brief 将偏移数组转换为对应的索引数组
/// @param offsets 偏移数组
/// @param size_of_offsets 偏移数组大小
/// @param[out] indices 索引数组
/// @param size_of_indices 索引数组大小
template <memory_space_t space, typename index_t, typename offset_t>
void offsets_to_indices(const index_t* offsets,
                        offset_t const& size_of_offsets,
                        offset_t* indices,
                        index_t const& size_of_indices) {
  // 使用设备空间时选择0号CUDA流执行策略,反之使用主机执行策略
  using execution_policy_t = std::conditional_t<space == memory_space_t::device,
                                                decltype(thrust::cuda::par.on(
                                                    0)),  // XXX: does this
                                                          // work on stream 0?
                                                decltype(thrust::host)>;
  execution_policy_t exec;
  // convert compressed offsets into uncompressed indices
  thrust::fill(exec, indices + 0, indices + size_of_indices, offset_t(0));

  // `thrust::counting_iterator`:计数迭代器,类似Python的range(),表示一个连续范围序列的迭代器
  // `thrust::zip_iterator`:打包迭代器,可以同时对多个数组同时遍历
  // `thrust::transform_iterator`:转换迭代器,访问函数时调用转换算子进行转换
  // `thrust::scatter_if(exec,first,last,map,stencil,output)`:
  //   根据映射将元素从源范围有条件地复制到输出数组中.
  //   对于范围[first，last)中的每个迭代器i,若stencil[i-first]为true,则将值*i分配给output[map[i-first]]
  // 将偏移值散列到索引的最高位置
  // 如:offsets[0, 2, 2, 3, 5, 5, 5, 7, 8]得到indices[0, 0, 2, 3, 0, 6, 0, 7]
  thrust::scatter_if(
      exec,                                    // execution policy
      thrust::counting_iterator<offset_t>(0),  // begin iterator
      thrust::counting_iterator<offset_t>(size_of_offsets - 1),  // end iterator
      offsets + 0,  // where to scatter
      thrust::make_transform_iterator(  // 用于判断相邻两个元素是否相等
          thrust::make_zip_iterator(    // 用于遍历两个相邻元素
              thrust::make_tuple(offsets + 0, offsets + 1)),
          [=] __host__ __device__(const thrust::tuple<offset_t, offset_t>& t) {
            thrust::not_equal_to<offset_t> comp;
            return comp(thrust::get<0>(t), thrust::get<1>(t));
          }),
      indices + 0);

  // `thrust::inclusive_scan()`:前缀运算
  // 前缀最大值运算,将上述操作散列的一个索引值分散成多个,从而转换成索引
  // 如:indices[0, 0, 2, 3, 0, 6, 0, 7]得到indices[0, 0, 2, 3, 3, 6, 6, 7]
  thrust::inclusive_scan(exec, indices + 0, indices + size_of_indices,
                         indices + 0, thrust::maximum<offset_t>());
}

/// @brief 将索引数组转换为对应的偏移数组
/// @param indices 索引数组
/// @param size_of_indices 索引数组大小
/// @param[out] offsets 偏移数组
/// @param size_of_offsets 偏移数组大小
template <memory_space_t space, typename index_t, typename offset_t>
void indices_to_offsets(const index_t* indices,
                        index_t const& size_of_indices,
                        offset_t* offsets,
                        offset_t const& size_of_offsets) {
  using execution_policy_t =
      std::conditional_t<space == memory_space_t::device,
                         decltype(thrust::device), decltype(thrust::host)>;
  execution_policy_t exec;
  // `thrust::lower_bound(exec,first,last,values_first,values_last,result)`:
  //   对[values_first,values_last)中的每个值,找到在[first,last)中的可插入的下界索引
  // convert uncompressed indices into compressed offsets
  // 如:indices[0, 0, 2, 3, 3, 6, 6, 7]得到offsets[0, 2, 2, 3, 5, 5, 5, 7, 8]
  thrust::lower_bound(exec, indices, indices + size_of_indices,
                      thrust::counting_iterator<offset_t>(0),
                      thrust::counting_iterator<offset_t>(size_of_offsets),
                      offsets + 0);
}

}  // namespace convert
}  // namespace graph
}  // namespace gunrock
