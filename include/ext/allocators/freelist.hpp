#ifndef INCLGUARD_free_list_allocator_hpp
#define INCLGUARD_free_list_allocator_hpp

#include "detail_block.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace ext::allocoators {
#if 0
// TODO: finish this allocator
//#    error unfinished

template<typename ParentAllocator,
         std::size_t ChunkSize,
         std::size_t MinSize,
         std::size_t MaxSize,
         std::size_t MaxElements>
class free_list_allocator : ParentAllocator {
    private:
    struct free_block {
        free_block* next;
        std::size_t size;
    };

    public:
    free_list_allocator() : _head(nullptr) {}

    static constexpr actual_size(std::size_t size) {
        return ((size - 1) / ChunkSize + 1) * ChunkSize;
    }

    memory_block allocate(std::size_t alignment, std::size_t size) {
        if (_head && MinSize <= size && size <= MaxSize && _head) {
            char* ptr = _head;
            std::size_t space = ChunkSize;
            if (boost::alignment::align(alignment, size, ptr, space)) {
                _head = _head->next;
                return {ptr, space};
            }
        }

        return ParentAllocator::allocate(alignment, actual_size(size));
    }

    // TODO: implement reallocate
    // void reallocate(memory_block& block, std::size_t new_size);

    void deallocate(memory_block block) {
        assert(owns(block));
        boost::alignment::aligned_free(block.data);
    }

    bool owns(memory_block block) const {
        return ParentAllocator::owns(block);
    }

    private:
    free_block* _head;
};
#endif
} // namespace ext::allocoators

#endif
