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
		memblock allocate(std::size_t size)
		{
			auto block = PrimaryAllocator::allocate(size);
			if(!block.ptr)
				block = FallbackAllocator::allocate(size);

			return block;
		}

		void deallocate(memblock block)
		{
			if(PrimaryAllocator::owns(block))
				PrimaryAllocator::deallocate(block);
			else
				FallbackAllocator::deallocate(block);
		}

		bool owns(memblock block)
		{
			return PrimaryAllocator::owns(block) || FallAllocator::owns(block);
		}
	};

} // alloc

#endif

