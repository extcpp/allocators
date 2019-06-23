#ifndef INCLGUARD_standard_allocator_hpp
#define INCLGUARD_standard_allocator_hpp

#include "memblock.hpp"
#include <boost/align/aligned_alloc.hpp>
#include <cassert>
#include <cstddef>

namespace alloc {
class standard_allocator {
    public:
    static constexpr std::size_t actual_size(std::size_t size, std::size_t) noexcept {
        return size;
    }

    memblock allocate(std::size_t size, std::size_t alignment) {
        auto* ptr = boost::alignment::aligned_alloc(alignment, size);
        if (ptr)
            return {ptr, size};
        else
            return {nullptr, 0};
    }

    // TODO: implement reallocate
    // void reallocate(memblock& block, std::size_t new_size);

    void deallocate(memblock block) {
        assert(owns(block));
        boost::alignment::aligned_free(block.ptr);
    }

    /**
        \note This function always returns true
    */
    bool owns(memblock) const noexcept {
        return true;
    }
};
} // namespace alloc

#endif
