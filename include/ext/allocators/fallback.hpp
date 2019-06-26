#ifndef INCLGUARD_fallback_allocator_hpp
#define INCLGUARD_fallback_allocator_hpp

#include "detail_block.hpp"
#include "detail_traits.hpp"
#include <tuple>

namespace ext::allocoators {
namespace _detail_fallback_allocator {
template<typename Derived, typename FirstAllocator, typename SecondAllocator, typename = void>
struct extension_allocate_array {};

template<typename Derived, typename FirstAllocator, typename SecondAllocator>
struct extension_allocate_array<
    Derived,
    FirstAllocator,
    SecondAllocator,
    std::enable_if_t<_detail::has_allocate_array_v<FirstAllocator> && _detail::has_allocate_array_v<SecondAllocator>>> {

    template<typename OutItr>
    std::tuple<OutItr, bool>
        allocate_array(std::size_t alignment, std::size_t size, std::size_t count, OutItr out_itr) {
        auto* parent = static_cast<Derived*>(this);
        auto result = static_cast<FirstAllocator*>(parent)->allocate_array(alignment, size, count, out_itr);
        if (!std::get<1>(result))
            result = static_cast<SecondAllocator*>(parent)->allocate_array(alignment, size, count, out_itr);

        return result;
    }
};
} // namespace _detail_fallback_allocator

template<typename FirstAllocator, typename SecondAllocator>
class fallback_allocator
    : public _detail_fallback_allocator::
          extension_allocate_array<fallback_allocator<FirstAllocator, SecondAllocator>, FirstAllocator, SecondAllocator>
    , private FirstAllocator
    , private SecondAllocator {
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

} // namespace ext::allocoators

#endif
