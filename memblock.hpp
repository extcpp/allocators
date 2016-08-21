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
} // namespace alloc

#endif
