#ifndef INCLGUARD_size_segregator_hpp
#define INCLGUARD_size_segregator_hpp

#include <cstddef>
#include "memblock.hpp"

namespace alloc
{
	template<typename FirstAllocator, typename SecondAllocator, std::size_t SizeLessOrEqual>
	struct size_segregator : private FirstAllocator, private SecondAllocator
	{
		using first_allocator = FirstAllocator;
		using second_allocator = SecondAllocator;

		static constexpr std::size_t dividing_size = SizeLessOrEqual;

		static constexpr std::size_t actual_size(std::size_t size, std::size_t alignment)
		{
			if(size <= dividing_size)
				return FirstAllocator::actual_size(size, alignment);
			else
				return SecondAllocator::actual_size(size, alignment);
		}

		/// allocates memory of given size and alignment
		/**
			\return Returns memblock on success, which denotes the memory and size of the allocation,
			        a nullptr and 0 size otherwise.

			\note This function is a requirement.
		*/
		memblock allocate(std::size_t size, std::size_t alignment)
		{
			if(size <= dividing_size)
				return FirstAllocator::allocate(size, alignment);
			else
				return SecondAllocator::allocate(size, alignment);
		}

		/// deallocates the memory denoted by the given memblock
		/**
			\note The given memblock object must previously be obtained by an allocate function of *this.
			      Passing in a memblock object, which was not previously obtained by an allocate function
			      of *this leads to undefined behavior.

			\note This function is a requirement.
		*/
		void deallocate(memblock block)
		{
			if(block.size <= dividing_size)
				return FirstAllocator::deallocate(block);
			else
				return SecondAllocator::deallocate(block);
		}

		/// returns true if the given memblock object denotes memory allocated by *this, false otherwise
		/**
			\note This function is a requirement.
		*/
		bool owns(memblock block) const
		{
			if(block.size <= dividing_size)
				return FirstAllocator::owns(block);
			else
				return SecondAllocator::owns(block);
		}
	}
}

#endif
