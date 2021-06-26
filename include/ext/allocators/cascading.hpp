#ifndef EXT_ALLOCATORS_CASCADING_HEADER
#define EXT_ALLOCATORS_CASCADING_HEADER

#include "memory_block.hpp"
#include "stl_wrapper.hpp"

#include <cassert>
#include <cstddef>
#include <tuple>
#include <vector>

namespace EXT_ALLOCATOR_NAMESPACE {

/// creates ChildAllocator objects in the memory pool of ParentAllocator as needed

template<typename ParentAllocator, typename ChildAllocator>
class cascading_allocator : private ParentAllocator
{
public:
    using parent_allocator_t = ParentAllocator;
    using child_allocator_t = ChildAllocator;

    cascading_allocator() : _chunks{allocator_wrapper<child_allocator_t, parent_allocator_t>{this}} {}
    cascading_allocator(cascading_allocator const&) = delete;
    cascading_allocator& operator=(cascading_allocator const&) = delete;
    cascading_allocator(cascading_allocator&&) noexcept = default;
    cascading_allocator& operator=(cascading_allocator&&) = default;

    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) {
        return ChildAllocator::actual_size(alignment, size);
    }

    bool owns(memory_block block) const {
        return find(block) != _chunks.end();
    }

    memory_block allocate(std::size_t alignment, std::size_t size) {
        memory_block block{nullptr, 0};
        allocate_impl(alignment, size, 1, &block);
        return block;
    }

    void deallocate(memory_block block) {
        auto itr = find(block);
        assert(itr != _chunks.end());
        itr->deallocate(block);
    }

private:
    template<typename OutItr>
    std::tuple<OutItr, bool> allocate_impl(std::size_t alignment, std::size_t size, std::size_t count, OutItr out_itr) {
        std::tuple<OutItr, bool> result{out_itr, false};
        for (auto itr = _chunks.begin(); itr != _chunks.end(); ++itr) {
            result = allocate_helper(*itr, alignment, size, count, out_itr);
            if (std::get<1>(result))
                return result;
        }

        try {
            _chunks.emplace_back();
            result = allocate_helper(_chunks.back(), alignment, size, count, out_itr);
        } catch (std::bad_alloc&) {
        }

        return result;
    }

    auto find(memory_block block) const {
        return std::find_if(_chunks.begin(), _chunks.end(), [&](auto& a) { return a->owns(block); });
    }

    auto find(memory_block block) {
        return std::find_if(_chunks.begin(), _chunks.end(), [&](auto& a) { return a.owns(block); });
    }

    std::vector<child_allocator_t, allocator_wrapper<child_allocator_t, parent_allocator_t>> _chunks;
};
} // namespace EXT_ALLOCATOR_NAMESPACE

#endif // EXT_ALLOCATORS_CASCADING_HEADER
