#ifndef INCLGUARD_bitmap_allocator_hpp
#define INCLGUARD_bitmap_allocator_hpp

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <boost/align/aligned_alloc.hpp>
#include <boost/integer.hpp>
#include "memblock.hpp"

namespace alloc
{
	template<typename ParentAllocator, std::size_t ChunkSize, std::size_t NumChunks, std::size_t Alignment>
	class bitmap_allocator : ParentAllocator
	{
	public:
		static_assert(NumChunks > 0, "NumChunks must be greater than 0");

		static constexpr std::size_t chunk_size = ChunkSize;
		static constexpr std::size_t num_chunks = NumChunks;
		static constexpr std::size_t memory_alignment = Alignment;
		static constexpr std::size_t small_chunk_size = SmallChunkSize;
		static constexpr std::size_t free_blocks_size = ((NumChunks - 1) / 64) + 1;

		bitmap_allocator() : _memory{nullptr, 0}
		{ }

		~bitmap_allocator()
		{
			ParentAllocator::deallocate(_memory);
		}

		memblock allocate(std::size_t size, std::size_t alignment)
		{
			memblock out{nullptr, 0};
			allocate_array(size, alignment, 1, &out);
			return out;
		}

		template<typename OutItr>
		OutItr allocate_array(std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr)
		{
			if(!_memory.ptr)
				init();

			if(0 < size && size * count <= std::numeric_limits<std::uint64_t>::digits() * chunk_size && _memory.ptr)
			{
				std::size_t sub_length = pattern_length(size);
				std::size_t length = sub_length * count;
				std::size_t max_pos = free_blocks_size * 64 - length;
				std::uint64_t pat = pattern(length);

				std::size_t pos = 0;
				while(pos < max_pos && check_position(pos, pat))
					++pos;

				if(pos < max_pos)
				{
					auto shift = pattern_shift(pos);
					auto index = free_blocks_index(pos);
					_free_blocks[index]     &= ~(pat <<       shift );
					_free_blocks[index + 1] &= ~(pat >> (64 - shift));

					for(std::size_t i=0; i<count; ++i)
						*out_itr++ = memblock{_memory + pos * chunk_size + i * sub_length * chunk_size, sub_length * chunk_size};
				}
			}

			return out_itr;
		}

		void deallocate(memblock block)
		{
			if(owns(block))
			{
				std::size_t pos = (block.ptr - _memory.ptr) / chunk_size;
				_free_blocks[free_blocks_index(block.size)] |= pattern(pattern_length(block.size)) << pattern_shift(pos);
			}
		}

		bool owns(memblock block)
		{
			return block.ptr >= _memory.ptr && block.ptr + block.size <= _memory.ptr + _memory.size;
		}

	private:
		void init()
		{
			_memory = ParentAllocator::allocate(chunk_size * num_chunks, memory_alignment);
			std::fill(_free_blocks, _free_blocks + free_blocks_size, ~std::uint64_t{0});
			_free_blocks[free_blocks_size] = 0; // sentinel (note: free_blocks_size is 1 less the actual array)
		}

		bool check_position(std::size_t pos, std::uint64_t pattern) const
		{
			std::size_t index = free_blocks_index(pos);
			std::size_t shift = pattern_shift(pos);

			return (_free_blocks[index]     & (pattern <<       shift) ) == (pattern <<       shift )
			    && (_free_blocks[index + 1] & (pattern >> (64 - shift))) == (pattern >> (64 - shift));
		}

		static std::size_t pattern_length(std::size_t memory_size)
		{
			return (size - 1) / chunk_size + 1;
		}

		static std::size_t pattern(std::size_t length)
		{
			return (std::uint64_t{1} << (length + 1)) - 1;
		}

		static std::size_t pattern_shift(std::size_t chunk_index)
		{
			return chunk_index & 0b111111;
		}

		static std::size_t free_blocks_index(std::size_t chunk_index)
		{
			return chunk_index >> 6;
		}

	private:
		memblock _memory;
		std::uint64_t _free_blocks[free_blocks_size + 1]; // +1 for simpler check_position function
	};
}

#endif
