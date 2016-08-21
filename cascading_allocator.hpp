#ifndef INCLGUARD_cascading_allocator_hpp
#define INCLGUARD_cascading_allocator_hpp

#include <cassert>
#include <cstddef>
#include "memblock.hpp"
#include "allocator_wrapper.hpp"

namespace alloc
{
	/// creates ChildAllocator objects in the memory pool of ParentAllocator as needed
	template<typename ParentAllocator, typename ChildAllocator>
	class cascading_allocator : ParentAllocator
	{
	public:
		using parent_allocator_t = ParentAllocator;
		using child_allocator_t = ChildAllocator;

		cascading_allocator()
			: _chunks{allocator_wrapper<child_allocator_t, parent_allocator_t>{this}}
		{ }

		memblock allocate(std::size_t size, std::size_t alignment)
		{
			memblock block{nullptr, 0};

			for(auto itr=_chunks.begin(); itr!=_chunks.end(); ++itr)
			{
				block = itr->allocate(size, alignment);
				if(block.ptr)
					return block;
			}

			try
			{
				_chunks.emplace_back();
				block = _chunks.back().allocate(size, alignment);
			}
			catch(std::bad_alloc&)
			{ }

			return block;
		}

		void deallocate(memblock block)
		{
			auto itr = find(block);
			assert(itr != _chunks.end());
			itr->deallocate(block);
		}

		bool owns(memblock block) const
		{
			return find(block) != _chunks.end();
		}

	private:
		auto find(memblock block) const
		{
			return std::find_if(_chunks.begin(), _chunks.end(), [&](auto& a){
				return a->owns(block);
			});
		}

	private:
		std::vector<child_allocator_t, allocator_wrapper<child_allocator_t, parent_allocator_t>> _chunks;
	}
}

#endif
