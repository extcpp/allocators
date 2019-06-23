#ifndef INCLGUARD_memblock_hpp
#define INCLGUARD_memblock_hpp

#include <cstddef>

namespace alloc {
struct memblock {
    using pointer = void*;
    using size_t = std::size_t;

    memblock() noexcept = default;

    memblock(void* ptr, std::size_t size) : ptr{ptr}, size{size} {}

    memblock(std::nullptr_t) : ptr(nullptr), size{0} {}

    explicit operator bool() {
        return ptr != nullptr;
    }

    void* ptr;
    std::size_t size;
};

bool operator==(memblock lhs, memblock rhs) {
    return lhs.ptr == rhs.ptr && lhs.size == rhs.size;
}

bool operator!=(memblock lhs, memblock rhs) {
    return !(lhs == rhs);
}

struct deleter_options {
    enum { divergent_size = 0b01, local = 0b10 };
};


template<typename Allocator, int Options = 0>
struct deleter {
    using allocator_t = Allocator;

    template<typename Type>
    void operator()(Type* ptr) const {
        static_assert(Allocator::actual_size(sizeof(Type), alignof(Type)) == sizeof(Type),
                      "allocator must be able to allocate exactly the size of given type");

        if (ptr) {
            ptr->~Type();
            allocator_t::instance().deallocate(memblock{ptr, sizeof(Type)});
        }
    }
};

template<typename Allocator>
struct deleter<Allocator, deleter_options::local> {
    using allocator_t = Allocator;

    template<typename Type>
    void operator()(Type* ptr) const {
        static_assert(Allocator::actual_size(sizeof(Type), alignof(Type)) == sizeof(Type),
                      "allocator must be able to allocate exactly the size of given type");

        if (ptr) {
            ptr->~Type();
            allocator->deallocate(memblock{ptr, sizeof(Type)});
        }
    }

    allocator_t* allocator;
};

template<typename Allocator>
struct deleter<Allocator, deleter_options::divergent_size> {
    using allocator_t = Allocator;

    template<typename Type>
    void operator()(Type* ptr) const {
        if (ptr) {
            ptr->~Type();
            allocator_t::instance().deallocate(memblock{ptr, size});
        }
    }

    memblock::size_t size;
};

template<typename Allocator>
struct deleter<Allocator, deleter_options::divergent_size | deleter_options::local> {
    using allocator_t = Allocator;

    template<typename Type>
    void operator()(Type* ptr) const {
        if (ptr) {
            ptr->~Type();
            allocator->deallocate(memblock{ptr, size});
        }
    }

    allocator_t* allocator;
    memblock::size_t size;
};


/// returns a unique_ptr holding the given type created with a global instance of Allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(std::nullptr_t, Args&&... args)
    -> std::enable_if_t<Allocator::actual_size(sizeof(Type), alignof(Type)) == sizeof(Type),
                        std::unique_ptr<Type, deleter<Allocator>>> {
    auto block = Allocator::instance().allocate(sizeof(Type), alignof(Type));
    return std::unique_ptr<Type, deleter<Allocator>>{new (block.ptr) Type{std::forward<Args>(args)...}};
}

/// returns a unique_ptr holding the given type created with a global instance of Allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(std::nullptr_t, Args&&... args)
    -> std::enable_if_t<Allocator::actual_size(sizeof(Type), alignof(Type)) != sizeof(Type),
                        std::unique_ptr<Type, deleter<Allocator, deleter_options::divergent_size>>> {
    auto block = Allocator::instance().allocate(sizeof(Type), alignof(Type));
    return {new (block.ptr) Type{std::forward<Args>(args)...}, {block.size}};
}

/// returns a unique_ptr holding the given type created with the given allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(Allocator* a, Args&&... args)
    -> std::enable_if_t<Allocator::actual_size(sizeof(Type), alignof(Type)) == sizeof(Type),
                        std::unique_ptr<Type, deleter<Allocator, deleter_options::local>>> {
    auto block = a->allocate(sizeof(Type), alignof(Type));
    return {new (block.ptr) Type{std::forward<Args>(args)...}, {a}};
}

/// returns a unique_ptr holding the given type created with the given allocator
template<typename Type, typename Allocator, typename... Args>
auto make_unique(Allocator* a, Args&&... args) -> std::enable_if_t<
    Allocator::actual_size(sizeof(Type), alignof(Type)) != sizeof(Type),
    std::unique_ptr<Type, deleter<Allocator, deleter_options::divergent_size | deleter_options::local>>> {
    auto block = a->allocate(sizeof(Type), alignof(Type));
    return {new (block.ptr) Type{std::forward<Args>(args)...}, {a, block.size}};
}

} // namespace alloc

#endif
