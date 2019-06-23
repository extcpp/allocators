#ifndef INCLGUARD_allocator_traits_hpp
#define INCLGUARD_allocator_traits_hpp

#include "detail_block.hpp"
#include <type_traits>

namespace alloc { namespace _detail {

template<typename T, typename = void>
struct has_allocate_array : std::false_type {};

template<typename T>
struct has_allocate_array<T,
                          std::void_t<decltype(std::declval<T>().allocate_array(
                              std::size_t{}, std::size_t{}, std::size_t{}, (memory_block*){}))>> : std::true_type {};

template<typename T>
inline bool has_allocate_array_v = has_allocate_array<T>::value;

}} // namespace alloc::_detail

#endif
