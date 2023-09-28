#pragma once

#include <type_traits>

namespace std {

#if __cplusplus == 201402L
// Supported in C++ 17 (https://en.cppreference.com/w/cpp/types/disjunction)
template <class...>
struct disjunction : std::false_type {};
template <class B1>
struct disjunction<B1> : B1 {};
template <class B1, class... Bn>
struct disjunction<B1, Bn...>
    : std::conditional_t<bool(B1::value), B1, disjunction<Bn...>> {};

template <class... B>
constexpr bool disjunction_v =
    disjunction<B...>::value;  // C++17 supports inline constexpr bool

// Supported in C++ 17 (https://en.cppreference.com/w/cpp/types/is_arithmetic)
template <class T>
constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

// Supported in C++ 17
// (https://en.cppreference.com/w/cpp/types/is_floating_point)
template <class T>
constexpr bool is_floating_point_v = is_floating_point<T>::value;
#endif
}  // namespace std

namespace gunrock {

/**
 * @note The following solution is extracted from:
 * https://stackoverflow.com/q/67347545/5729690, purpose of the following few
 * structs are to filter certain types from a given tuple. It will be nice if
 * the parameter pack from the tuple could then be extracted out (but, idk how
 * to do this right now).
 */
template <typename...>
struct tuple_join {};
/// @brief 合并两个tuple生成一个合并的tuple类型
template <typename... Pack1, typename... Pack2>
struct tuple_join<std::tuple<Pack1...>, std::tuple<Pack2...>> {
  using type = std::tuple<Pack1..., Pack2...>;
};

template <typename...>
struct filter_single_tuple {};

/// @brief 过滤tuple中的单个类型(递归终止条件)
///        Pack为空时,即std::tuple<>调用,返回std::tuple<>::type
template <typename Target, typename... Pack>
struct filter_single_tuple<Target, std::tuple<Pack...>> {
  using type = std::tuple<Pack...>;
};

/// @brief 过滤tuple中的单个类型
/// @tparam Target 要过滤的类型
/// @tparam Parameter tuple中的第一个模板参数类型
/// @tparam ...Pack tuple中的后续模板参数类型
template <typename Target, typename Parameter, typename... Pack>
struct filter_single_tuple<Target, std::tuple<Parameter, Pack...>> {
  // 合并元组类型
  using type = typename tuple_join<
      // 若Parameter类型与Target类型相同则从std::tuple<Parameter, Pack...>中移除
      std::conditional_t<std::is_same_v<Target, Parameter>,
                         std::tuple<>,
                         std::tuple<Parameter>>,
      // 递归判断Pack...中剩余类型
      typename filter_single_tuple<Target, std::tuple<Pack...>>::type
  >::type;
};

template <typename... Pack>
struct filter_tuple {};
/// @brief 过滤tuple中指定类型(递归终止条件)
template <typename... Types>
struct filter_tuple<std::tuple<>, std::tuple<Types...>> {
  using type = std::tuple<Types...>;
};

/// @brief 过滤tuple中指定类型
/// @tparam Target 待过滤掉的第一个模板参数类型
/// @tparam ...RemainingTargets 剩余待过滤掉的模板参数类型
/// @tparam ...Types 需要被过滤的模板类型
template <typename Target, typename... RemainingTargets, typename... Types>
struct filter_tuple<std::tuple<Target, RemainingTargets...>,
                    std::tuple<Types...>> {
  using type = typename filter_single_tuple<
      // 从Types...Target
      Target,
      // 从Types递归去除RemainingTargets...
      typename filter_tuple<std::tuple<RemainingTargets...>,
                            std::tuple<Types...>
                            >::type
  >::type;
};

/**
 * @brief Takes two tuple types, A and B, finds tuple types C, where C = B - A.
 * Result is std::tuple<C>.
 *
 * @tparam tuple_with_types_to_filter_t
 * @tparam apply_filter_to_this_tuple_t
 */
template <typename tuple_with_types_to_filter_t,
          typename apply_filter_to_this_tuple_t>
using filter_tuple_t =
    typename filter_tuple<tuple_with_types_to_filter_t,
                          apply_filter_to_this_tuple_t>::type;

}  // namespace gunrock