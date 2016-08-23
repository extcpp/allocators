#ifndef INCLGUARD_memblock_hpp
#define INCLGUARD_memblock_hpp

#include <cstddef>

namespace alloc
{
	struct memblock
	{
		using pointer = void*;
		using size_t = std::size_t;

		memblock() noexcept = default;

		memblock(void* ptr, std::size_t size)
			: ptr{ptr}, size{size}
		{ }

		memblock(std::nullptr_t)
			: ptr(nullptr), size{0}
		{ }

		explicit operator bool()
		{
			return ptr != nullptr;
		}

		void* ptr;
		std::size_t size;
	};

	bool operator== (memblock lhs, memblock rhs)
	{
		return lhs.ptr == rhs.ptr && lhs.size == rhs.size;
	}

	bool operator!= (memblock lhs, memblock rhs)
	{
		return !(lhs == rhs);
	}


	template<typename Allocator>
	struct static_size_memblock_deleter
	{
		using allocator_t = Allocator;

		template<typename Type>
		void operator() (Type* ptr) const
		{
			if(ptr)
			{
				ptr->~Type();
				allocator->deallocate(memblock{ptr, sizeof(Type)});
			}
		}

		allocator_t* allocator;
	};

	template<typename Allocator>
	struct memblock_deleter
	{
		using allocator_t = Allocator;

		template<typename Type>
		void operator() (Type* ptr) const
		{
			if(ptr)
			{
				ptr->~Type();
				allocator->deallocate(memblock{ptr, size});
			}
		}

		allocator_t* allocator;
		memblock::size_t size;
	};


	template<typename Type, typename Allocator, typename... Args>
	std::unique_ptr<Type, memblock_deleter<Allocator>>
	make_unique(Allocator& a, std::size_t alignment, Args... args)
	{
		auto block = a->allocate(sizeof(Type), alignment);
		return {new (block.ptr) Type{args...}, {&a, block.size} };
	}

} // namespace alloc

#endif
