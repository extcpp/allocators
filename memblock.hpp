#ifndef INCLGUARD_memblock_hpp
#define INCLGUARD_memblock_hpp

#include <cstddef>

namespace alloc
{
	struct memblock
	{
		void* ptr;
		std::size_t size;
	};

} // namespace alloc

#endif
