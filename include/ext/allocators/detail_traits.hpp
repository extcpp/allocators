#ifndef INCLGUARD_allocator_traits_hpp
#define INCLGUARD_allocator_traits_hpp

#include "detail_block.hpp"
#include <type_traits>
#    include "config.hpp"

namespace EXT_ALLOCATOR_NAMESPACE { namespace _detail {

template<typename T, typename = void>
struct has_allocate_array : std::false_type {};

inline memory_block* traits_helper{};
template<typename T>
struct has_allocate_array<
    T,
    std::void_t<decltype(std::declval<T>().allocate_array(std::size_t{}, std::size_t{}, std::size_t{}, traits_helper))>>
    : std::true_type {};

template<typename T>
inline bool has_allocate_array_v = has_allocate_array<T>::value;

}} // namespace EXT_ALLOCATOR_NAMESPACE::_detail

#endif
