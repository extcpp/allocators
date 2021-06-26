#ifndef EXT_ALLOCATORS_FALLBACK_HEADER
#define EXT_ALLOCATORS_FALLBACK_HEADER

#include "memory_block.hpp"
#include <tuple>
#include <limits>

namespace EXT_ALLOCATOR_NAMESPACE {

template<typename FirstAllocator, typename SecondAllocator>
class fallback_allocator
    : private FirstAllocator
    , private SecondAllocator
{
public:
    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) {
        return FirstAllocator::actual_size(alignment, size) == SecondAllocator::actual_size(alignment, size)
                   ? FirstAllocator::actual_size(alignment, size)
                   : std::numeric_limits<std::size_t>::max();
    }

    bool owns(memory_block block) const {
        return FirstAllocator::owns(block) || SecondAllocator::owns(block);
    }

    memory_block allocate(std::size_t size, size_t alignment) {
        auto block = FirstAllocator::allocate(alignment, size);
        if (!block.data) {
            block = SecondAllocator::allocate(alignment, size);
        }

        return block;
    }

    void deallocate(memory_block block) {
        if (FirstAllocator::owns(block)) {
            FirstAllocator::deallocate(block);
        } else {
            SecondAllocator::deallocate(block);
        }
    }
};

} // namespace EXT_ALLOCATOR_NAMESPACE

#endif // EXT_ALLOCATORS_FALLBACK_HEADER
