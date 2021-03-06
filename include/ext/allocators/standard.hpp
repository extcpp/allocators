#ifndef EXT_ALLOCATORS_STANDARD_HEADER
#define EXT_ALLOCATORS_STANDARD_HEADER

#include "detail_block.hpp"
#include <cassert>
#include <cstdlib>

namespace EXT_ALLOCATOR_NAMESPACE {
class standard_allocator {
    public:
    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) noexcept {
        (void) alignment;
        return size;
    }

    bool owns(memory_block) const noexcept {
        return true;
    }

    memory_block allocate(std::size_t alignment, std::size_t size) {
        auto* data = (std::byte*) std::aligned_alloc(alignment, size);
        if (data) {
            return {data, size};
        } else {
            return {nullptr, 0};
        }
    }

    void deallocate(memory_block block) {
        assert(owns(block));
        std::free(block.data);
    }
};
} // namespace EXT_ALLOCATOR_NAMESPACE

#endif // EXT_ALLOCATORS_STANDARD_HEADER
