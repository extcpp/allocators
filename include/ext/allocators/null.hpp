#pragma once
#ifndef INCLGUARD_null_allocator_hpp
#    define INCLGUARD_null_allocator_hpp

#    include "detail_block.hpp"
#    include <cassert>

namespace EXT_ALLOCATOR_NAMESPACE {
struct null_allocator {
    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) {
        (void) alignment;
        (void) size;
        return 0;
    }

    bool owns(memory_block block) const {
        return block.data == nullptr;
    }

    memory_block allocate(std::size_t /* alignment*/, std::size_t /* size */) {
        return {nullptr, 0};
    }

    void deallocate(memory_block block) {
        (void) block;
        assert(block.data == nullptr);
    }

    // bool expand(memory_block&, std::size_t /* new_size */) {
    //    return false;
    //}

    memory_block allocate_all(std::size_t /* alignment */) {
        return {nullptr, 0};
    }

    void deallocate_all() {}
};
} // namespace EXT_ALLOCATOR_NAMESPACE

#endif
