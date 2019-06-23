#ifndef INCLGUARD_bitmap_allocator_hpp
#define INCLGUARD_bitmap_allocator_hpp

#include "detail_block.hpp"

#include <cassert>
#include <tuple>

namespace alloc {
template<typename ParentAllocator, std::size_t ChunkSize, std::size_t NumChunks, std::size_t Alignment>
class bitmap_allocator : ParentAllocator {
    public:
    static_assert(NumChunks > 0, "NumChunks must be greater than 0");

    static constexpr std::size_t chunk_size = ChunkSize;
    static constexpr std::size_t num_chunks = NumChunks;
    static constexpr std::size_t memory_alignment = Alignment;
    static constexpr std::size_t free_blocks_size = ((NumChunks - 1) / 64) + 1;

    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) noexcept {
        (void) alignment;
        return pattern_length(size) * chunk_size;
    }

    bitmap_allocator() noexcept : _block{nullptr, 0} {}

    bitmap_allocator(bitmap_allocator const&) = delete;
    bitmap_allocator& operator=(bitmap_allocator const&) = delete;

    bitmap_allocator(bitmap_allocator&& other) noexcept {
        _block = other._block;
        other._block = {nullptr, 0};

        for (std::size_t i = 0; i < free_blocks_size; ++i) _free_blocks[i] = other._free_blocks[i];
    }

    bitmap_allocator& operator=(bitmap_allocator&& other) noexcept {
        _block = other._block;
        other._block = {nullptr, 0};

        for (std::size_t i = 0; i < free_blocks_size; ++i) _free_blocks[i] = other._free_blocks[i];

        return *this;
    }

    ~bitmap_allocator() {
        if (_block.data)
            ParentAllocator::deallocate(_block);
    }

    memory_block allocate(std::size_t alignment, std::size_t size) {
        memory_block out{nullptr, 0};
        allocate_array(alignment, size, 1, &out);
        return out;
    }

    template<typename OutItr>
    std::tuple<OutItr, bool>
        allocate_array(std::size_t alignment, std::size_t size, std::size_t count, OutItr out_itr) {
        if (!_block.data)
            init();

        bool success = false;
        if (pattern_length(size) <= 128 && size * count <= num_chunks * chunk_size && 0 < size && _block.data) {
            std::size_t sub_length = pattern_length(size);
            std::size_t length = sub_length * count;
            std::size_t max_pos = num_chunks - length;
            std::uint64_t pat = pattern(length);

            std::size_t pos = 0;
            while (check_position(pos, length, alignment) == false && pos <= max_pos) ++pos;

            if (pos <= max_pos) {
                auto shift = pattern_shift(pos);
                auto index = free_blocks_index(pos);
                _free_blocks[index] &= ~(pat << shift);
                if ((shift & 63) + length > 64)
                    _free_blocks[index + 1] &= ~(pat >> (64 - shift));

                for (std::size_t i = 0; i < count; ++i)
                    *out_itr++ = memory_block{reinterpret_cast<std::byte*>(_block.data) + pos * chunk_size +
                                                  i * sub_length * chunk_size,
                                              sub_length * chunk_size};

                success = true;
            }
        }

        return std::tuple<OutItr, bool>{out_itr, success};
    }

    void deallocate(memory_block block) noexcept {
        assert(owns(block));
        std::size_t pos = (reinterpret_cast<char*>(block.data) - reinterpret_cast<char*>(_block.data)) / chunk_size;
        _free_blocks[free_blocks_index(block.size)] |= pattern(pattern_length(block.size)) << pattern_shift(pos);
    }

    bool owns(memory_block block) const noexcept {
        return reinterpret_cast<char*>(block.data) >= _block.data &&
               reinterpret_cast<char*>(block.data) + block.size <= reinterpret_cast<char*>(_block.data) + _block.size;
    }

    private:
    void init() {
        _block = ParentAllocator::allocate(chunk_size * num_chunks, memory_alignment);
        std::fill(_free_blocks, _free_blocks + free_blocks_size - 1, ~std::uint64_t{0});

        auto remaining = num_chunks - 64 * (free_blocks_size - 1);
        if (remaining < 64)
            _free_blocks[free_blocks_size - 1] = pattern(remaining);
        else
            _free_blocks[free_blocks_size - 1] = ~std::uint64_t{0};
    }

    bool check_position(std::size_t pos, std::size_t length, std::size_t alignment) const noexcept {
        // check alignment of position
        if (((reinterpret_cast<intptr_t>(_block.data) + pos * chunk_size) & (alignment - 1)) != 0)
            return false;

        std::size_t index = free_blocks_index(pos);
        std::size_t shift = pattern_shift(pos);
        std::uint64_t pat = pattern(length);

        bool out = (_free_blocks[index] & (pat << shift)) == (pat << shift);
        if ((shift & 63) + length > 64)
            out = out && (_free_blocks[index + 1] & (pat >> (64 - shift))) == (pat >> (64 - shift));

        return out;
    }

    static constexpr std::size_t pattern_length(std::size_t memory_size) noexcept {
        return (memory_size - 1) / chunk_size + 1;
    }

    static constexpr std::size_t pattern(std::size_t length) noexcept {
        return (std::uint64_t{1} << length) - 1;
    }

    static constexpr std::size_t pattern_shift(std::size_t chunk_index) noexcept {
        return chunk_index & 0b111111;
    }

    static constexpr std::size_t free_blocks_index(std::size_t chunk_index) noexcept {
        return chunk_index >> 6;
    }

    private:
    memory_block _block;
    std::uint64_t _free_blocks[free_blocks_size];
};
} // namespace alloc

#endif
