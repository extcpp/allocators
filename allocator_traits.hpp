#ifndef INCLGUARD_allocator_traits_hpp
#define INCLGUARD_allocator_traits_hpp

#include "void.hpp"

namespace alloc
{
	namespace _detail
	{
		#define CREATE_HAS_FUNCTION(name) \
			template<typename T, typename = void> struct has_function_##name : std::false_type { }; \
			template<typename T> struct has_function_##name <T, utility::void_t<decltype(T::name)>> : std::true_type { };

		CREATE_HAS_FUNCTION(allocate_all);
		CREATE_HAS_FUNCTION(deallocate_all);
		CREATE_HAS_FUNCTION(allocate_array);

		#undef CREATE_HAS_FUNCTION
	} // namespace

	template<typename Allocator>
	struct allocator_traits
	{
		static constexpr bool has_allocate_all =
			   _detail::has_function_allocate_all<Allocator>::value
			&& _detail::has_function_deallocate_all<Allocator>::value;

		static constexpr bool has_allocate_array =
			   _detail::has_function_allocate_array<Allocator>::value;
	};

} // namespace alloc

#endif