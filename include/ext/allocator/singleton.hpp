#ifndef INCLGUARD_singleton_allocator_hpp
#define INCLGUARD_singleton_allocator_hpp

#include "detail_block.hpp"
#include "detail_traits.hpp"
#include <cassert>
#include <cstddef>

namespace alloc {
namespace _detail_singleton_allocator {
template<typename Derived, typename Allocator, typename = void>
struct extension_allocate_array {};

template<typename Derived, typename Allocator>
struct extension_allocate_array<Derived, Allocator, std::enable_if_t<_detail::has_allocate_array_v<Allocator>>> {
    template<typename OutItr>
    std::tuple<OutItr, bool>
        allocate_array(std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr) {
        auto* parent = static_cast<Derived*>(this);
        return parent->instance().allocate_array(size, alignment, count, out_itr);
    }
};
} // namespace _detail_singleton_allocator

/// creates a thread_local instance of the given allocator type, which will used by any instance of this allocator
/**
    \note The instance of Allocator will be created as a thread_local static function variable.
*/
template<typename Allocator, typename Tag = void>
struct singleton_allocator
    : public _detail_singleton_allocator::extension_allocate_array<singleton_allocator<Allocator>, Allocator> {
    using allocator_t = Allocator;

    /// returns the actual size for the requested size and alignment
    /**
        \note If the size cannot be determined at compile time, the returned size is
       std::numeric_limits<std::size_t>::max()
    */
    static constexpr std::size_t actual_size(std::size_t size, std::size_t alignment) {
        return Allocator::actual_size(size, alignment);
    }

    /// returns the global instance of allocator_t
    static allocator_t& instance() {
        thread_local static allocator_t a;
        return a;
    }

    /// allocates memory of given size and alignment
    /**
        \return Returns memory_block on success, which denotes the memory and size of the allocation,
                a nullptr and 0 size otherwise.

        \note This function is a requirement.
    */
    memory_block allocate(std::size_t size, std::size_t alignment) {
        return instance().allocate(size, alignment);
    }

    /// deallocates the memory denoted by the given memory_block
    /**
        \note The given memory_block object must previously be obtained by an allocate function of *this.
              Passing in a memory_block object, which was not previously obtained by an allocate function
              of *this leads to undefined behavior.

        \note This function is a requirement.
    */
    void deallocate(memory_block block) {
        return instance().deallocate(block);
    }

    /// returns true if the given memory_block object denotes memory allocated by *this, false otherwise
    /**
        \note This function is a requirement.
    */
    bool owns(memory_block block) const {
        return instance().owns(block);
    }
};
} // namespace alloc

#endif
