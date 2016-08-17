#ifndef INCLGUARD_blob_allocator_hpp
#define INCLGUARD_blob_allocator_hpp

#include <cassert>
#include <cstddef>
#include <boost/align/aligned_alloc.hpp>
#include "memblock.hpp"

namespace alloc
{
	template<std::size_t MemorySize>
	class blob_allocator
	{
	public:
		static constexpr std::size_t memory_size = MemorySize;

		blob_allocator() : allocated{false}
		{ }

		memblock allocate(std::size_t size, std::size_t alignment)
		{
			memblock out{nullptr, 0};
			if(size <= memory_size)
				out = allocate_all(alignment);

			return out;
		}

		memblock allocate_all(std::size_t alignment)
		{
			memblock out{nullptr, 0};

			if(!allocated)
			{
				auto* ptr = boost::alignment::align_up(alignment, _data)
				if(ptr < _data + Size)
				{
					out = {ptr, _data + Size - ptr};
					allocated = true;
				}
			}

			return out;
		}

		void deallocate(memblock block)
		{
			if(owns(block))
				allocated = false;
		}

		void deallocate_all()
		{
			allocated = false;
		}

		bool owns(memblock block)
		{
			return block.ptr >= data && block.ptr + block.size <= data + MemorySize;
		}

	private:
		char data[MemorySize];
		bool allocated;
	};
}

#endif
