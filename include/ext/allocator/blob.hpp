#ifndef INCLGUARD_blob_allocator_hpp
#define INCLGUARD_blob_allocator_hpp

#include "detail_block.hpp"
#include <boost/align/aligned_alloc.hpp>
#include <cassert>
#include <cstddef>

namespace alloc {
template<std::size_t MemorySize, std::size_t Alignment>
class alignas(Alignment) blob_allocator {
    public:
    static constexpr std::size_t memory_size = MemorySize;
    static constexpr std::size_t memory_alignment = Alignment;

    static constexpr std::size_t actual_size(std::size_t size, std::size_t alignment) noexcept {
        return size <= memory_size && alignment <= memory_alignment ? memory_size : 0;
    }

    blob_allocator() noexcept : _allocated{false} {}

    blob_allocator(blob_allocator const&) = delete;
    blob_allocator(blob_allocator&&) = delete;


    memory_block allocate(std::size_t size, std::size_t alignment) noexcept {
        memory_block out{nullptr, 0};
        if (size <= memory_size)
            out = allocate_all(alignment);

        return out;
    }

    memory_block allocate_all(std::size_t alignment) noexcept {
        memory_block out{nullptr, 0};

        if (!_allocated && alignment <= memory_alignment) {
            out = {_data, memory_size};
            _allocated = true;
        }

        return out;
    }

    void deallocate(memory_block block) noexcept {
        (void) block;
        assert(owns(block));
        _allocated = false;
    }

    void deallocate_all() noexcept {
        _allocated = false;
    }

    bool owns(memory_block block) const noexcept {
        return block.data >= _data && reinterpret_cast<char*>(block.data) + block.size <= _data + memory_size;
    }

    private:
    char _data[memory_size];
    bool _allocated;
};
} // namespace alloc

#endif
