#ifndef INCLGUARD_allocator_traits_hpp
#define INCLGUARD_allocator_traits_hpp

#include "void.hpp"
#include "memblock.hpp"

namespace alloc
{
	namespace _detail
	{
		template<typename T, typename = void> struct has_allocate_array : std::false_type { };
		template<typename T> struct has_allocate_array<T, utility::void_t<decltype(std::declval<T>().allocate_array(std::size_t{}, std::size_t{}, std::size_t{}, (memblock*){}))>> : std::true_type { };
	} // namespace

	template<typename Allocator>
	struct allocator_traits
	{
		static constexpr bool has_allocate_array =
			   _detail::has_allocate_array<Allocator>::value;
	};

} // namespace alloc

#endif
