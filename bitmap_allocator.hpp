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
		static constexpr std::size_t free_blocks_size = ((NumChunks - 1) / 64) + 1;

		static constexpr std::size_t actual_size(std::size_t size, std::size_t alignment) noexcept
		{
			return alignment <= memory_alignment ? pattern_length(size) + chunk_size : 0;
		}

		bitmap_allocator() noexcept : _memory{nullptr, 0}
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

			if(pattern_length(size) <= 128 && size * count <= num_chunks * chunk_size && 0 < size && _memory.ptr)
			{
				std::size_t sub_length = pattern_length(size);
				std::size_t length = sub_length * count;
				std::size_t max_pos = num_chunks - length;
				std::uint64_t pat = pattern(length);

				std::size_t pos = 0;
				while(check_position(pos, length, alignment) == false && pos <= max_pos)
					++pos;

				if(pos <= max_pos)
				{
					auto shift = pattern_shift(pos);
					auto index = free_blocks_index(pos);
					_free_blocks[index] &= ~(pat << shift);
					if((shift & 63) + length > 64)
						_free_blocks[index + 1] &= ~(pat >> (64 - shift));

					for(std::size_t i=0; i<count; ++i)
						*out_itr++ = memblock{reinterpret_cast<char*>(_memory.ptr) + pos * chunk_size + i * sub_length * chunk_size, sub_length * chunk_size};
				}
			}

			return out_itr;
		}

		void deallocate(memblock block) noexcept
		{
			assert(owns(block));
			std::size_t pos = (reinterpret_cast<char*>(block.ptr) - reinterpret_cast<char*>(_memory.ptr)) / chunk_size;
			_free_blocks[free_blocks_index(block.size)] |= pattern(pattern_length(block.size)) << pattern_shift(pos);
		}

		bool owns(memblock block) const noexcept
		{
			return reinterpret_cast<char*>(block.ptr) >= _memory.ptr
				&& reinterpret_cast<char*>(block.ptr) + block.size <= reinterpret_cast<char*>(_memory.ptr) + _memory.size;
		}

	private:
		void init()
		{
			_memory = ParentAllocator::allocate(chunk_size * num_chunks, memory_alignment);
			std::fill(_free_blocks, _free_blocks + free_blocks_size - 1, ~std::uint64_t{0});

			auto remaining = num_chunks - 64 * (free_blocks_size - 1);
			if(remaining < 64)
				_free_blocks[free_blocks_size - 1] = pattern(remaining);
			else
				_free_blocks[free_blocks_size - 1] = ~std::uint64_t{0};
		}

		bool check_position(std::size_t pos, std::size_t length, std::size_t alignment) const noexcept
		{
			// check alignment of position
			if( ((reinterpret_cast<intptr_t>(_memory.ptr) + pos * chunk_size) & (alignment - 1)) != 0 )
				return false;

			std::size_t index = free_blocks_index(pos);
			std::size_t shift = pattern_shift(pos);
			std::uint64_t pat = pattern(length);

			bool out =  (_free_blocks[index] & (pat << shift)) == (pat << shift);
			if((shift & 63) + length > 64)
			    out = out && (_free_blocks[index + 1] & (pat >> (64 - shift))) == (pat >> (64 - shift));

			return out;
		}

		static constexpr std::size_t pattern_length(std::size_t memory_size) noexcept
		{
			return (memory_size - 1) / chunk_size + 1;
		}

		static constexpr std::size_t pattern(std::size_t length) noexcept
		{
			return (std::uint64_t{1} << length) - 1;
		}

		static constexpr std::size_t pattern_shift(std::size_t chunk_index) noexcept
		{
			return chunk_index & 0b111111;
		}

		static constexpr std::size_t free_blocks_index(std::size_t chunk_index) noexcept
		{
			return chunk_index >> 6;
		}

	private:
		memblock _memory;
		std::uint64_t _free_blocks[free_blocks_size];
	};
}

#endif
