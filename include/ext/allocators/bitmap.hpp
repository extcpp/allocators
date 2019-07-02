#ifndef INCLGUARD_bitmap_allocator_hpp
#define INCLGUARD_bitmap_allocator_hpp

#include "detail_block.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <tuple>
#include <stdexcept>

namespace ext::allocoators {
template<typename ParentAllocator, std::size_t Alignment, std::size_t ChunkSize, std::size_t NumChunks>
class bitmap_allocator : ParentAllocator {


    public:
    static_assert(NumChunks > 0, "NumChunks must be greater than 0");

    static constexpr std::size_t bits = sizeof(uintptr_t) * 8;
    static_assert(bits == 64, "Your pointer does not have 64bit!!!");

    static constexpr std::size_t chunk_size = ChunkSize;
    static constexpr std::size_t num_chunks = NumChunks;
    static constexpr std::size_t memory_alignment = Alignment;
    static constexpr std::size_t free_blocks_size = ((NumChunks - 1) / bits) + 1;

    bitmap_allocator() noexcept : _block{nullptr, 0} {}
    bitmap_allocator(bitmap_allocator const&) = delete;
    bitmap_allocator& operator=(bitmap_allocator const&) = delete;

    bitmap_allocator(bitmap_allocator&& other) noexcept {
        _block = other._block;
        other._block = {nullptr, 0};
        std::copy(other._free_blocks.begin(), other._free_blocks.end(), _free_blocks.begin());
    }

    bitmap_allocator& operator=(bitmap_allocator&& other) noexcept {
        _block = other._block;
        other._block = {nullptr, 0};
        std::copy(other._free_blocks.begin(), other._free_blocks.end(), _free_blocks.begin());
        return *this;
    }

    ~bitmap_allocator() {
        if(_block) {
            ParentAllocator::deallocate(_block);
        }
    }

    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) noexcept {
        assert(alignment <= Alignment); (void) alignment;
        return pattern_length(size) * chunk_size;
    }

    bool owns(memory_block block) const noexcept {
        return _block.owns(block);
    }

    memory_block allocate(std::size_t alignment, std::size_t size) {
        memory_block out{nullptr, 0};
        allocate_array(alignment, size, 1, &out);
        return out;
    }

    void deallocate(memory_block block) noexcept {
        assert(owns(block));
        auto diff = (block.data - _block.data);
        assert(diff >= 0);
        std::size_t pos = static_cast<std::size_t>(diff) / chunk_size;
        _free_blocks[free_blocks_index(block.size)] |= pattern(pattern_length(block.size)) << pattern_shift(pos);
    }

    template<typename OutItr>
    std::tuple<OutItr, bool>
        allocate_array(std::size_t alignment, std::size_t size, std::size_t count, OutItr out_itr) {
        if (!_block.data) {
            init();
        }

        bool success = false;
        if (pattern_length(size) <= 128 && size * count <= num_chunks * chunk_size && 0 < size && _block.data) {
            std::size_t sub_length = pattern_length(size);
            std::size_t length = sub_length * count;
            std::size_t max_pos = num_chunks - length;
            std::uint64_t pat = pattern(length);

            std::size_t pos = 0;
            while (check_position(pos, length, alignment) == false && pos <= max_pos) {
                ++pos;
            }

            if (pos <= max_pos) {
                auto shift = pattern_shift(pos);
                auto index = free_blocks_index(pos);
                _free_blocks[index] &= ~(pat << shift);
                if ((shift & 63) + length > bits) {
                    _free_blocks[index + 1] &= ~(pat >> (bits - shift));
                }

                auto const sub_chunk_length = sub_length * chunk_size;
                for (std::size_t i = 0; i < count; ++i) {
                    *out_itr++ = memory_block{reinterpret_cast<std::byte*>(_block.data) + pos * chunk_size +
                                                  i * sub_chunk_length,
                                              sub_chunk_length};
                }

                success = true;
            }
        }
        return std::tuple<OutItr, bool>{out_itr, success};
    }


    private:
    void init() {
        // get memory from the parent allocator
        _block = ParentAllocator::allocate(memory_alignment, chunk_size * num_chunks);

        // mark all blocks as free
        std::fill(_free_blocks.begin(), _free_blocks.end(), ~std::uint64_t{0});

        constexpr auto remaining = num_chunks - bits * (free_blocks_size - 1);
        if (remaining < bits) {
            _free_blocks.back() = pattern(remaining);
        } else {
            _free_blocks.back() = ~std::uint64_t{0};
        }
    }

    bool check_position(std::size_t pos, std::size_t length, std::size_t alignment) const noexcept {
        // check alignment of position
        if (((reinterpret_cast<intptr_t>(_block.data) + intptr_t(pos * chunk_size)) & intptr_t(alignment - 1)) != 0) {
            return false;
        }

        std::size_t index = free_blocks_index(pos);
        std::size_t shift = pattern_shift(pos);
        std::uint64_t pat = pattern(length);

        bool out = (_free_blocks[index] & (pat << shift)) == (pat << shift);
        if ((shift & 63) + length > bits) {
            out = out && (_free_blocks[index + 1] & (pat >> (bits - shift))) == (pat >> (bits - shift));
        }

        return out;
    }

    static constexpr std::size_t pattern_length(std::size_t memory_size) noexcept {
        return (memory_size - 1) / chunk_size + 1;
    }

    static constexpr std::size_t pattern(std::size_t length) noexcept {
        return (std::uintptr_t{1} << length) - 1;
    }

    static constexpr std::size_t pattern_shift(std::size_t chunk_index) noexcept {
        return chunk_index & 0b111111;
    }

    static constexpr std::size_t free_blocks_index(std::size_t chunk_index) noexcept {
        return chunk_index >> 6;
    }

    private:
    memory_block _block; // internal allocation made by ParentAllocator
    std::array<std::uintptr_t, free_blocks_size> _free_blocks;
};
} // namespace ext::allocoators

#endif
