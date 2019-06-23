#pragma once
#ifndef INCLGUARD_null_allocator_hpp
#    define INCLGUARD_null_allocator_hpp

#    include "detail_block.hpp"
#    include <cassert>

namespace alloc {
struct null_allocator {
    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) {
        (void) alignment;
        (void) size;
        return 0;
    }

    memory_block allocate(std::size_t /* alignment*/, std::size_t /* size */) {
        return {nullptr, 0};
    }

    memory_block allocate_all(std::size_t /* alignment */) {
        return {nullptr, 0};
    }

    bool expand(memory_block& /* block */, std::size_t /* new_size */) {
        return false;
    }

    void reallocate(memory_block& /* block */, std::size_t /* new_size */) {}

    void deallocate(memory_block block) {
        (void) block;
        assert(block.data == nullptr);
    }

    /// deallocates all memory
    void deallocate_all() {}

    bool owns(memory_block block) const {
        return block.data == nullptr;
    }
};
} // namespace alloc

#endif
