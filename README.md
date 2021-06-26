# Simple Allocation Lib

## Basics

### Memory Blocks

A `memory_block` describes any allocation made by any of the allocators. It consists of an
`address` and `size`.

### STL Wrapper

`stl_wrapper.hpp` provides means to make use of the allocators with container types from
standard template library.

## Common Allocator Functions

### Modifiers

```c++
    memory_block allocate(std::size_t alignment, std::size_t size) noexcept
```

Allocates memory of `size` and `alignment`.


```c++
    void deallocate(memory_block block) EXTALLOC_NOEXCEPT
```

Deallocates given `block`.

### Observers

```c++
    bool owns(memory_block block) EXTALLOC_NOEXCEPT
```

Returns true if `block` is owned by allocator, false otherwise.


```c++
    static constexpr std::size_t actual_size(std::size_t alignment, std::size_t size) noexcept
```

Returns the actual allocated size the allocator would allocate when `allocate`
is called with `size` and `alignment`.


## Not Always Implemented
TODO - check where this is meaningful

```c++
    memory_block allocate_all(std::size_t alignment) noexcept
```

Allocates as much memory of `alignment` as the allocator can provide.

```c++
    void deallocate_all() noexcept
```

Deallocates all memory owned by this allocator.

```c++
    template<typename OutItr> std::tuple<OutItr, bool>
    allocate_array(std::size_t alignment, std::size_t size, std::size_t count, OutItr out_itr)
```

Allocates multiple `memory_blocks` instead of a single block.
TODO - check if always continous and update wording or change function name

## List of Allocators

### Providing Allocators

Allocators in this group provide access to system's memory. They allocate with
`malloc`, by creating an object on the stack or even by using other allocation
libs.

#### Standard

This allocator uses `std::aligned_alloc` and `free` to provide memory.

Note: The standard allocator always claims to own any `memory_block`. Make sure to have asked all
other involved allocators before you deallocate using this allocator.

#### Blob

This allocator allocates a blob of memory in itself. So depending where it is
created it can provide memory on the stack or heap.

#### Null

This allocator always fails to allocate.

### Managing Allocators

Allocators in this group provide allocations strategies and manage
how the underlying pools are accessed.

#### Alignment Segregator

This allocator dispatches allocation requests to underlying allocators. When the
requested alignment is less or equal the `dividing_alignment`, then it will use
the `FirstAllocator` otherwise the `SecondAllocator`.

#### Size Segregator

This allocator dispatches allocation requests to underlying allocators. When the
requested size is less or equal the `dividing_size`. Then it will use the
`FirstAllocator` otherwise the `SecondAllocator`.

#### Fallback

This allocator dispatches allocation requests to the `FirstAllocator` as long
as it is able to provide valid `memory_blocks`. If the `FirstAllocator` is
exhausted it falls back to the `SecondAllocator`.

#### Stack (dumb - and therefor mostly short lived)

This allocator allocates and deallocates memory in a stack-like fashion. It is
meant to be used in small scopes, when memory is mostly just allocated and
freed at scope exit. This is because tracking the allocation order is often
infeasible in bigger scopes and the allocator needs to deallocate blocks in
an exact order for deallocation to work properly.

#### Bitmap

This allocator encodes in single bits which regions are allocated and which are not.
Therefore it is easy for this allocator to provide single allocations.

Note: Supports array allocation


### Special Allocators

#### Singleton
TODO

Is a singleton (global) allocator with `thread_local` memory really that useful?

### WIP Allocators

#### Cascading (WIP)

TODO

Has list of children and get memory from parent to create new children.
(how to keep tack of children that get empty)

#### Freelist (WIP)

TODO

liked list ... unallocated regions point to next unallocated regions ...
