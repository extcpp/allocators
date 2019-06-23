#ifndef INCLGUARD_cascading_allocator_hpp
#define INCLGUARD_cascading_allocator_hpp

#include "detail_block.hpp"
#include "detail_traits.hpp"
#include "stl_wrapper.hpp"

#include <cassert>
#include <cstddef>
#include <tuple>
#include <vector>

namespace alloc {
namespace _detail_cascading_allocator {
template<typename Derived, typename ChildAllocator, typename = void>
class extension_allocate_array {
    protected:
    template<typename OutItr>
    std::tuple<OutItr, bool> allocate_helper(
        ChildAllocator& allocator, std::size_t alignment, std::size_t size, std::size_t count, OutItr out_itr) {
        (void) count; // FIXME
        assert(count == 1);
        memory_block block = allocator.allocate(alignment, size);
        if (block.data) {
            *out_itr++ = block;
            return {out_itr, true};
        } else
            return {out_itr, false};
    }
};

template<typename Derived, typename ChildAllocator>
class extension_allocate_array<Derived,
                               ChildAllocator,
                               std::enable_if_t<_detail::has_allocate_array_v<ChildAllocator>>> {
    public:
    template<typename OutItr>
    std::tuple<OutItr, bool>
        allocate_array(std::size_t alignment, std::size_t size, std::size_t count, OutItr out_itr) {
        return static_cast<Derived*>(this)->allocate_impl(alignment, size, count, out_itr);
    }

    protected:
    template<typename OutItr>
    std::tuple<OutItr, bool> allocate_helper(
        ChildAllocator& allocator, std::size_t alignment, std::size_t size, std::size_t count, OutItr out_itr) {
        return allocator.allocate_array(alignment, size, count, out_itr);
    }
};
} // namespace _detail_cascading_allocator

/// creates ChildAllocator objects in the memory pool of ParentAllocator as needed
template<typename ParentAllocator, typename ChildAllocator>
class cascading_allocator
    : public _detail_cascading_allocator::extension_allocate_array<cascading_allocator<ParentAllocator, ChildAllocator>,
                                                                   ChildAllocator>
    , private ParentAllocator {
    private:
    using base =
        _detail_cascading_allocator::extension_allocate_array<cascading_allocator<ParentAllocator, ChildAllocator>,
                                                              ChildAllocator>;
    friend base;

    using base::allocate_helper;

    public:
    using parent_allocator_t = ParentAllocator;
    using child_allocator_t = ChildAllocator;

    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) {
        return ChildAllocator::actual_size(alignment, size);
    }

    cascading_allocator() : _chunks{allocator_wrapper<child_allocator_t, parent_allocator_t>{this}} {}

    cascading_allocator(cascading_allocator const&) = delete;
    cascading_allocator& operator=(cascading_allocator const&) = delete;

    cascading_allocator(cascading_allocator&&) noexcept = default;
    cascading_allocator& operator=(cascading_allocator&&) = default;

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

    bool owns(memory_block block) const {
        return find(block) != _chunks.end();
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

    private:
    std::vector<child_allocator_t, allocator_wrapper<child_allocator_t, parent_allocator_t>> _chunks;
};
} // namespace alloc

#endif
