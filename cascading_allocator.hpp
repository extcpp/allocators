#ifndef INCLGUARD_cascading_allocator_hpp
#define INCLGUARD_cascading_allocator_hpp

#include <cassert>
#include <cstddef>
#include "memblock.hpp"
#include "allocator_wrapper.hpp"

namespace alloc
{
	namespace _detail_cascading_allocator
	{
		template<typename Derived, typename ChildAllocator, typename = void>
		class extension_allocate_array
		{
		protected:
			OutItr allocate_helper(ChildAllocator& allocator, std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr)
			{
				assert(count == 1);
				*out_itr++ = allocator.allocate(size, alignment);
				return out_itr;
			}
		};

		template<typename Derived, typename ChildAllocator>
		class extension_allocate_array<Derived, ChildAllocator, std::enable_if_t<alloc::allocator_traits<ChildAllocator>::has_allocate_array>>
		{
		public:
			OutItr allocate_array(std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr)
			{
				return static_cast<Derived*>(this)->allocate_impl(size, alignment, count, out_itr);
			}

		protected:
			OutItr allocate_helper(ChildAllocator& allocator, std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr)
			{
				return allocator.allocate_array(size, alignment, count, out_itr);
			}
		};
	}

	/// creates ChildAllocator objects in the memory pool of ParentAllocator as needed
	template<typename ParentAllocator, typename ChildAllocator>
	class cascading_allocator :
		public _detail_cascading_allocator::extension_allocate_array<cascading_allocator, ChildAllocator>,
		private ParentAllocator
	{
	public:
		using parent_allocator_t = ParentAllocator;
		using child_allocator_t = ChildAllocator;

		static constexpr std::size_t actual_size(std::size_t size, std::size_t alignment)
		{
			return ChildAllocator::actual_size(size, alignment);
		}

		cascading_allocator()
			: _chunks{allocator_wrapper<child_allocator_t, parent_allocator_t>{this}}
		{ }

		memblock allocate(std::size_t size, std::size_t alignment)
		{
			memblock block{nullptr, 0};
			allocate_impl(size, alignment, 1, &block);
			return block;
		}

		void deallocate(memblock block)
		{
			auto itr = find(block);
			assert(itr != _chunks.end());
			itr->deallocate(block);
		}

		bool owns(memblock block) const
		{
			return find(block) != _chunks.end();
		}

	private:
		template<typename OutItr>
		OutItr allocate_impl(std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr)
		{
			OutItr new_out_itr = out_itr;
			for(auto itr=_chunks.begin(); itr!=_chunks.end(); ++itr)
			{
				new_out_itr = allocate_helper(*itr, size, alignment, count, out_itr);
				if(new_out_itr != out_itr)
					return new_out_itr;
			}

			try
			{
				_chunks.emplace_back();
				new_out_itr = allocate_helper(_chunks.back(), size, alignment, count, out_itr);
			}
			catch(std::bad_alloc&)
			{ }

			return new_out_itr;
		}

		auto find(memblock block) const
		{
			return std::find_if(_chunks.begin(), _chunks.end(), [&](auto& a){
				return a->owns(block);
			});
		}

	private:
		std::vector<child_allocator_t, allocator_wrapper<child_allocator_t, parent_allocator_t>> _chunks;
	}
}

#endif
