#ifndef INCLGUARD_memblock_hpp
#define INCLGUARD_memblock_hpp

#include <cstddef>

namespace alloc
{
	struct memblock
	{
		using pointer = void*;
		using size_t = std::size_t;

		void* ptr;
		std::size_t size;
	};

} // namespace alloc

#endif
