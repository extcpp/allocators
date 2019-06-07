#ifndef INCLGUARD_null_allocator_hpp
#define INCLGUARD_null_allocator_hpp

#include <cassert>
#include <cstddef>
#include "memblock.hpp"

namespace alloc
{
	struct null_allocator
	{
		/// returns the actual size for the requested size and alignment
		/**
			\note If the size cannot be determined at compile time, the returned size is std::numeric_limits<std::size_t>::max()
		*/
		static constexpr std::size_t actual_size(std::size_t size, std::size_t alignment)
		{
			return 0;
		}

		/// allocates memory of given size and alignment
		/**
			\return Returns memblock on success, which denotes the memory and size of the allocation,
			        a nullptr and 0 size otherwise.

			\note This function is a requirement.
		*/
		memblock allocate(std::size_t /* size */, std::size_t /* alignment */)
		{
			return {nullptr, 0};
		}

		/// allocates the maximum amount of memory with the given alignment
		/**
			\return Returns memblock on success, which denotes the memory and size of the allocation,
			        a nullptr and 0 size otherwise.
		*/
		memblock allocate_all(std::size_t /* alignment */)
		{
			return {nullptr, 0};
		}

		bool expand(memblock& /* block */, std::size_t /* new_size */)
		{
			return false;
		}

		void reallocate(memblock& /* block */, std::size_t /* new_size */)
		{ }

		/// deallocates the memory denoted by the given memblock
		/**
			\note The given memblock object must previously be obtained by an allocate function of *this.
			      Passing in a memblock object, which was not previously obtained by an allocate function
			      of *this leads to undefined behavior.

			\note This function is a requirement.
		*/
		void deallocate(memblock block)
		{
			assert(block.ptr == nullptr);
		}

		/// deallocates all memory
		void deallocate_all()
		{ }

		/// returns true if the given memblock object denotes memory allocated by *this, false otherwise
		/**
			\note This function is a requirement.
		*/
		bool owns(memblock block) const
		{
			return block.ptr == nullptr;
		}
	}
}

#endif
