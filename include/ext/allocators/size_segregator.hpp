#ifndef EXT_ALLOCATORS_SIZE_SEGREGATOR_HEADER
#define EXT_ALLOCATORS_SIZE_SEGREGATOR_HEADER

#include "memory_block.hpp"
#include <type_traits>

namespace EXT_ALLOCATOR_NAMESPACE {

template<typename FirstAllocator, typename SecondAllocator, std::size_t SizeLessOrEqual>
struct size_segregator
    : private FirstAllocator
    , private SecondAllocator {

     using first_allocator = FirstAllocator;
     using second_allocator = SecondAllocator;

    static constexpr std::size_t dividing_size = SizeLessOrEqual;

    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) {
        if (size <= dividing_size) {
            return FirstAllocator::actual_size(alignment, size);
        } else {
            return SecondAllocator::actual_size(alignment, size);
        }
    }

    bool owns(memory_block block) const {
        if (block.size <= dividing_size) {
            return FirstAllocator::owns(block);
        } else {
            return SecondAllocator::owns(block);
        }
    }

    memory_block allocate(std::size_t alignment, std::size_t size) {
        if (size <= dividing_size) {
            return FirstAllocator::allocate(alignment, size);
        } else {
            return SecondAllocator::allocate(alignment, size);
        }
    }

    void deallocate(memory_block block) {
        if (block.size <= dividing_size) {
            return FirstAllocator::deallocate(block);
        } else {
            return SecondAllocator::deallocate(block);
        }
    }
};
} // namespace EXT_ALLOCATOR_NAMESPACE

#endif // EXT_ALLOCATORS_SIZE_SEGREGATOR_HEADER
