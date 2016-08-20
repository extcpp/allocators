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

		blob_allocator() : _allocated{false}
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

			if(!_allocated)
			{
				char* ptr = reinterpret_cast<char*>(boost::alignment::align_up(_data, alignment));
				if(ptr < _data + memory_size)
				{
					out = {ptr, static_cast<memblock::size_t>((_data + memory_size) - ptr)};
					_allocated = true;
				}
			}

			return out;
		}

		void deallocate(memblock block)
		{
			if(owns(block))
				_allocated = false;
		}

		void deallocate_all()
		{
			_allocated = false;
		}

		bool owns(memblock block) const
		{
			return block.ptr >= _data && reinterpret_cast<char*>(block.ptr) + block.size <= _data + memory_size;
		}

	private:
		char _data[memory_size];
		bool _allocated;
	};
}

#endif
