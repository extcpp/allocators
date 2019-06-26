#ifndef INCLGUARD_alignment_segregator_hpp
#define INCLGUARD_alignment_segregator_hpp

#include "detail_block.hpp"
#include "detail_traits.hpp"
#include <type_traits>

namespace ext::allocoators {
namespace _detail {
template<typename Derived,
         typename FirstAllocator,
         typename SecondAllocator,
         std::size_t AlignmentLessOrEqual,
         typename = void>
struct extension_allocate_array {};

template<typename Derived, typename FirstAllocator, typename SecondAllocator, std::size_t AlignmentLessOrEqual>
struct extension_allocate_array<
    Derived,
    FirstAllocator,
    SecondAllocator,
    AlignmentLessOrEqual,
    std::enable_if_t<_detail::has_allocate_array_v<FirstAllocator> && _detail::has_allocate_array_v<SecondAllocator>>> {
    template<class OutItr>
    std::tuple<OutItr, bool>
        allocate_array(std::size_t alignment, std::size_t size, std::size_t count, OutItr out_itr) {
        auto* parent = static_cast<Derived*>(this);
        if (alignment <= AlignmentLessOrEqual) {
            return static_cast<FirstAllocator*>(parent)->allocate_array(alignment, size, count, out_itr);
        } else {
            return static_cast<SecondAllocator*>(parent)->allocate_array(alignment, size, count, out_itr);
        }
    }
};
} // namespace _detail

template<typename FirstAllocator, typename SecondAllocator, std::size_t AlignmentLessOrEqual>
struct alignment_segregator
    : public _detail::extension_allocate_array<
          alignment_segregator<FirstAllocator, SecondAllocator, AlignmentLessOrEqual>,
          FirstAllocator,
          SecondAllocator,
          AlignmentLessOrEqual>
    , private FirstAllocator
    , private SecondAllocator {

    // using first_allocator = FirstAllocator;
    // using second_allocator = SecondAllocator;
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
} // namespace ext::allocoators

#endif
