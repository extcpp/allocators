#ifndef INCLGUARD_free_list_allocator_hpp
#define INCLGUARD_free_list_allocator_hpp

#include <cstdint>
#include <cassert>
#include <cstddef>
#include "memblock.hpp"
#include "if.hpp"

namespace alloc
{
	template<typename ParentAllocator, std::size_t ChunkSize, std::size_t MinSize, std::size_t MaxSize, std::size_t MaxElements>
	class free_list_allocator : ParentAllocator
	{
	private:
		struct free_block
		{
			free_block* next;
			std::size_t size;
		};

	public:
		free_list_allocator() : _head(nullptr)
		{ }

		static constexpr actual_size(std::size_t size)
		{
			return ((size / ChunkSize) + 1) * ChunkSize;
		}

		memblock allocate(std::size_t size, std::size_t alignment)
		{
			if(_head && MinSize <= size && size <= MaxSize && _head )
			{
				char* ptr = _head;
				std::size_t space = ChunkSize;
				if(boost::alignment::align(alignment, size, ptr, space))
				{
					_head =_head->next;
					return {ptr, space};
				}
			}

			return ParentAllocator::allocate(actual_size(size), alignment);
		}

		// TODO: implement reallocate
		//void reallocate(memblock& block, std::size_t new_size);

		void deallocate(memblock block)
		{
			assert(owns(block));
			boost::alignment::aligned_free(block.ptr);
		}

		bool owns(memblock block) const
		{
			return ParentAllocator::owns(block);
		}

	private:
		char* _head;
	};
} // namespace alloc

#endif
