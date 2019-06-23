#pragma once
#ifndef INCLGUARD_standard_allocator_hpp
#    define INCLGUARD_standard_allocator_hpp

#    include "detail_block.hpp"
#    include <cassert>
#    include <cstdlib>

namespace alloc {
class standard_allocator {
    public:
    static constexpr std::size_t actual_size(std::size_t size, std::size_t) noexcept {
        return size;
    }

    memory_block allocate(std::size_t size, std::size_t alignment) {
        auto* ptr = (std::byte*) std::aligned_alloc(alignment, size);
        if (ptr) {
            return {ptr, size};
        } else {
            return {nullptr, 0};
        }
    }

    void deallocate(memory_block block) {
        assert(owns(block));
        std::free(block.data);
    }

    bool owns(memory_block) const noexcept {
        return true;
    }
};
} // namespace alloc

#endif
