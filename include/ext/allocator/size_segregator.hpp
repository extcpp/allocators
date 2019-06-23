#ifndef INCLGUARD_size_segregator_hpp
#define INCLGUARD_size_segregator_hpp

#include "detail_block.hpp"
#include "detail_traits.hpp"
#include <type_traits>

namespace alloc {
namespace _detail_size_segregator {
template<typename Derived,
         typename FirstAllocator,
         typename SecondAllocator,
         std::size_t SizeLessOrEqual,
         typename = void>
struct extension_allocate_array {};

template<typename Derived, typename FirstAllocator, typename SecondAllocator, std::size_t SizeLessOrEqual>
struct extension_allocate_array<
    Derived,
    FirstAllocator,
    SecondAllocator,
    SizeLessOrEqual,
    std::enable_if_t<_detail::has_allocate_array_v<FirstAllocator> && _detail::has_allocate_array_v<SecondAllocator>>> {
    template<typename OutItr>
    std::tuple<OutItr, bool>
        allocate_array(std::size_t alignment, std::size_t size, std::size_t count, OutItr out_itr) {
        auto* parent = static_cast<Derived*>(this);
        if (size <= SizeLessOrEqual)
            return static_cast<FirstAllocator*>(parent)->allocate_array(alignment, size, count, out_itr);
        else
            return static_cast<SecondAllocator*>(parent)->allocate_array(alignment, size, count, out_itr);
    }
};
} // namespace _detail_size_segregator

template<typename FirstAllocator, typename SecondAllocator, std::size_t SizeLessOrEqual>
struct size_segregator
    : public _detail_size_segregator::extension_allocate_array<
          size_segregator<FirstAllocator, SecondAllocator, SizeLessOrEqual>,
          FirstAllocator,
          SecondAllocator,
          SizeLessOrEqual>
    , private FirstAllocator
    , private SecondAllocator {
    using first_allocator = FirstAllocator;
    using second_allocator = SecondAllocator;

    static constexpr std::size_t dividing_size = SizeLessOrEqual;

    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) {
        if (size <= dividing_size)
            return FirstAllocator::actual_size(alignment, size);
        else
            return SecondAllocator::actual_size(alignment, size);
    }

    /// allocates memory of given size and alignment
    /**
        \return Returns memory_block on success, which denotes the memory and size of the allocation,
                a nullptr and 0 size otherwise.

        \note This function is a requirement.
    */
    memory_block allocate(std::size_t alignment, std::size_t size) {
        if (size <= dividing_size)
            return FirstAllocator::allocate(alignment, size);
        else
            return SecondAllocator::allocate(alignment, size);
    }

    /// deallocates the memory denoted by the given memory_block
    /**
        \note The given memory_block object must previously be obtained by an allocate function of *this.
              Passing in a memory_block object, which was not previously obtained by an allocate function
              of *this leads to undefined behavior.

        \note This function is a requirement.
    */
    void deallocate(memory_block block) {
        if (block.size <= dividing_size)
            return FirstAllocator::deallocate(block);
        else
            return SecondAllocator::deallocate(block);
    }

    /// returns true if the given memory_block object denotes memory allocated by *this, false otherwise
    /**
        \note This function is a requirement.
    */
    bool owns(memory_block block) const {
        if (block.size <= dividing_size)
            return FirstAllocator::owns(block);
        else
            return SecondAllocator::owns(block);
    }
};
} // namespace alloc

#endif
