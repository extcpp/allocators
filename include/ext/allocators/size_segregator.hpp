#ifndef INCLGUARD_size_segregator_hpp
#define INCLGUARD_size_segregator_hpp

#include "detail_block.hpp"
#include "detail_traits.hpp"
#include <type_traits>

namespace EXT_ALLOCATOR_NAMESPACE {
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
        if (size <= SizeLessOrEqual) {
            return static_cast<FirstAllocator*>(parent)->allocate_array(alignment, size, count, out_itr);
        } else {
            return static_cast<SecondAllocator*>(parent)->allocate_array(alignment, size, count, out_itr);
        }
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
    // using first_allocator = FirstAllocator;
    // using second_allocator = SecondAllocator;

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

#endif
