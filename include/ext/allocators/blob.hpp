#ifndef EXT_ALLOCATORS_BLOB_HEADER
#define EXT_ALLOCATORS_BLOB_HEADER

#include "memory_block.hpp"
#include <cassert>
#include <cstddef>

namespace EXT_ALLOCATOR_NAMESPACE {
template<std::size_t Alignment, std::size_t MemorySize>

class alignas(Alignment) blob_allocator
{
public:
    static constexpr std::size_t memory_size = MemorySize;
    static constexpr std::size_t memory_alignment = Alignment;

    blob_allocator() noexcept : _allocated{false} {}
    blob_allocator(blob_allocator const&) = delete;
    blob_allocator(blob_allocator&&) = delete;

    memory_block allocate(std::size_t alignment, std::size_t size) noexcept {
        memory_block out{nullptr, 0};
        if (size <= memory_size)
            out = allocate_all(alignment);

        return out;
    }

    void deallocate(memory_block block) EXTALLOC_NOEXCEPT {
        (void) block;
        assert(owns(block));
        _allocated = false;
    }

    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) noexcept {
        return size <= memory_size && alignment <= memory_alignment ? memory_size : 0;
    }

    bool owns(memory_block block) EXTALLOC_NOEXCEPT {
        return owns_block(_data, memory_size, block);
    }

    memory_block allocate_all(std::size_t alignment) noexcept {
        memory_block out{nullptr, 0};

        if (!_allocated && alignment <= memory_alignment) {
            out = {_data, memory_size};
            _allocated = true;
        }

        return out;
    }

    void deallocate_all() noexcept {
        _allocated = false;
    }

private:
    std::byte _data[memory_size];
    bool _allocated;
};
} // namespace EXT_ALLOCATOR_NAMESPACE

#endif // EXT_ALLOCATORS_BLOB_HEADER
