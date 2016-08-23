#ifndef INCLGUARD_fallback_allocator_hpp
#define INCLGUARD_fallback_allocator_hpp

#include <cstddef>
#include "memblock.hpp"

namespace alloc
{
	namespace _detail_fallback_allocator
	{
		template<typename Derived, typename FirstAllocator, typename SecondAllocator, typename = void>
		struct extension_allocate_array
		{ };

		template<typename Derived, typename FirstAllocator, typename SecondAllocator>
		struct extension_allocate_array<Derived, FirstAllocator, SecondAllocator,
			std::enable_if_t<allocator_traits<FirstAllocator>::has_allocate_array && allocator_traits<SecondAllocator>::has_allocate_array>>
		{
			OutItr allocate_array(std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr)
			{
				auto* parent = static_cast<Derived*>(this);
				OutItr new_out_itr = static_cast<FirstAllocator*>(parent)->allocate_array(size, alignment, count, out_itr);
				if(new_out_itr == out_itr)
					new_out_itr = static_cast<SecondAllocator*>(parent)->allocate_array(size, alignment, count, out_itr);

				return new_out_itr;
			}
		};
	} // namespace _detail

	template<typename FirstAllocator, typename SecondAllocator>
	class fallback_allocator :
		public _detail_fallback_allocator::extension_allocate_array<fallback_allocator, FirstAllocator, SecondAllocator>,
		private FirstAllocator,
		private SecondAllocator
	{
	public:
		static constexpr std::size_t actual_size(std::size_t size, std::size_t alignment)
		{
			return
				FirstAllocator::actual_size(size, alignment) == SecondAllocator::actual_size(size, alignment)
				? FirstAllocator::actual_size(size, alignment)
				: std::numeric_limits<std::size_t>::max();
		}

		memblock allocate(std::size_t size, size_t alignment)
		{
			auto block = FirstAllocator::allocate(size, alignment);
			if(!block.ptr)
				block = SecondAllocator::allocate(size, alignment);

			return block;
		}

		void deallocate(memblock block)
		{
			if(FirstAllocator::owns(block))
				FirstAllocator::deallocate(block);
			else
				SecondAllocator::deallocate(block);
		}

		bool owns(memblock block) const
		{
			return FirstAllocator::owns(block) || SecondAllocator::owns(block);
		}
	};

} // alloc

#endif

