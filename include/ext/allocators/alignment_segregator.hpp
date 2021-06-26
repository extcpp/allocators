#ifndef EXT_ALLOCATORS_ALIGNMENT_SEGREGATOR_HEADER
#define EXT_ALLOCATORS_ALIGNMENT_SEGREGATOR_HEADER

#include "memory_block.hpp"
#include <type_traits>

namespace EXT_ALLOCATOR_NAMESPACE {

template<typename FirstAllocator, typename SecondAllocator, std::size_t AlignmentLessOrEqual>
struct alignment_segregator
    : private FirstAllocator
    , private SecondAllocator
{
    using first_allocator = FirstAllocator;
    using second_allocator = SecondAllocator;
    static constexpr std::size_t dividing_alignment = AlignmentLessOrEqual;

    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) {
        if (alignment <= dividing_alignment) {
            return FirstAllocator::actual_size(alignment, size);
        } else {
            return SecondAllocator::actual_size(alignment, size);
        }
    }

    bool owns(memory_block block) const {
        return FirstAllocator::owns(block) || SecondAllocator::owns(block);
    }

    memory_block allocate(std::size_t alignment, std::size_t size) {
        if (alignment <= dividing_alignment) {
            return FirstAllocator::allocate(alignment, size);
        } else {
            return SecondAllocator::allocate(alignment, size);
        }
    }

    void deallocate(memory_block block) {
        if (FirstAllocator::owns(block)) {
            return FirstAllocator::deallocate(block);
        } else {
            return SecondAllocator::deallocate(block);
        }
    }
};

} // namespace EXT_ALLOCATOR_NAMESPACE

#endif // EXT_ALLOCATORS_ALIGNMENT_SEGREGATOR_HEADER
