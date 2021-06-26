#ifndef EXT_ALLOCATORS_SINGLETON_HEADER
#define EXT_ALLOCATORS_SINGLETON_HEADER

#include "memory_block.hpp"
#include <cassert>
#include <cstddef>

namespace EXT_ALLOCATOR_NAMESPACE {

/// creates a thread_local instance of the given allocator type, which will used by any instance of this allocator
/**
    \note The instance of Allocator will be created as a thread_local static function variable.
*/
template<typename Allocator, typename Tag = void>
struct singleton_allocator {
    using allocator_t = Allocator;

    /// returns the global thread_local instance of allocator_t
    static allocator_t& instance() {
        thread_local static allocator_t a;
        return a;
    }

    /// returns the actual size for the requested size and alignment
    /**
        \note If the size cannot be determined at compile time, the returned size is
       std::numeric_limits<std::size_t>::max()
    */
    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) {
        return Allocator::actual_size(alignment, size);
    }

    bool owns(memory_block block) const {
        return instance().owns(block);
    }

    memory_block allocate(std::size_t alignment, std::size_t size) {
        return instance().allocate(alignment, size);
    }

    void deallocate(memory_block block) {
        return instance().deallocate(block);
    }
};
} // namespace EXT_ALLOCATOR_NAMESPACE

#endif // EXT_ALLOCATORS_SINGLETON_HEADER
