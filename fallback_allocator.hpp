#ifndef INCLGUARD_fallback_allocator_hpp
#define INCLGUARD_fallback_allocator_hpp

#include <cstddef>
#include "memblock.hpp"

namespace alloc
{
	template<typename FirstAllocator, typename SecondAllocator>
	class fallback_allocator : PrimaryAllocator, FallbackAllocator
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
			auto block = PrimaryAllocator::allocate(size, alignment);
			if(!block.ptr)
				block = FallbackAllocator::allocate(size, alignment);

			return block;
		}

		void deallocate(memblock block)
		{
			if(PrimaryAllocator::owns(block))
				PrimaryAllocator::deallocate(block);
			else
				FallbackAllocator::deallocate(block);
		}

		bool owns(memblock block) const
		{
			return PrimaryAllocator::owns(block) || FallbackAllocator::owns(block);
		}
	};

} // alloc

#endif

