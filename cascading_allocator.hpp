#ifndef INCLGUARD_cascading_allocator_hpp
#define INCLGUARD_cascading_allocator_hpp

#include <cassert>
#include <cstddef>
#include "memblock.hpp"

namespace alloc
{
	template<typename ParentAllocator, typename BaseAllocator>
	class cascading_allocator : ParentAllocator
	{
	public:
		memblock allocate(std::size_t size, std::size_t alignment)
		{
			return ;
		}

		void deallocate(memblock block)
		{
			assert(block.ptr == nullptr);
		}

		bool owns(memblock block) const
		{
			return block.ptr == nullptr;
		}

	private:
		std::unordered_map<char*, memblock> _chunks;
	}
}

#endif
