#ifndef INCLGUARD_cascading_allocator_hpp
#define INCLGUARD_cascading_allocator_hpp

#include <cassert>
#include <cstddef>
#include "allocator_traits.hpp"
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
			template<typename OutItr>
			std::tuple<OutItr, bool> allocate_helper(ChildAllocator& allocator, std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr)
			{
				assert(count == 1);
				memblock block = allocator.allocate(size, alignment);
				if(block.ptr)
				{
					*out_itr++ = block;
					return {out_itr, true};
				}
				else
					return {out_itr, false};
			}
		};

		template<typename Derived, typename ChildAllocator>
		class extension_allocate_array<Derived, ChildAllocator, std::enable_if_t<alloc::allocator_traits<ChildAllocator>::has_allocate_array>>
		{
		public:
			template<typename OutItr>
			std::tuple<OutItr, bool> allocate_array(std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr)
			{
				return static_cast<Derived*>(this)->allocate_impl(size, alignment, count, out_itr);
			}

		protected:
			template<typename OutItr>
			std::tuple<OutItr, bool> allocate_helper(ChildAllocator& allocator, std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr)
			{
				return allocator.allocate_array(size, alignment, count, out_itr);
			}
		};
	}

	/// creates ChildAllocator objects in the memory pool of ParentAllocator as needed
	template<typename ParentAllocator, typename ChildAllocator>
	class cascading_allocator :
		public _detail_cascading_allocator::extension_allocate_array<cascading_allocator<ParentAllocator, ChildAllocator>, ChildAllocator>,
		private ParentAllocator
	{
	private:
		using base = _detail_cascading_allocator::extension_allocate_array<cascading_allocator<ParentAllocator, ChildAllocator>, ChildAllocator>;
		friend base;

		using base::allocate_helper;

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

		cascading_allocator(cascading_allocator const&) = delete;
		cascading_allocator& operator=(cascading_allocator const&) = delete;

		cascading_allocator(cascading_allocator&&) noexcept = default;
		cascading_allocator& operator=(cascading_allocator&&) = default;

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
		std::tuple<OutItr, bool> allocate_impl(std::size_t size, std::size_t alignment, std::size_t count, OutItr out_itr)
		{
			std::tuple<OutItr, bool> result{out_itr, false};
			for(auto itr=_chunks.begin(); itr!=_chunks.end(); ++itr)
			{
				result = allocate_helper(*itr, size, alignment, count, out_itr);
				if(std::get<1>(result))
					return result;
			}

			try
			{
				_chunks.emplace_back();
				result = allocate_helper(_chunks.back(), size, alignment, count, out_itr);
			}
			catch(std::bad_alloc&)
			{ }

			return result;
		}

		auto find(memblock block) const
		{
			return std::find_if(_chunks.begin(), _chunks.end(), [&](auto& a){
				return a->owns(block);
			});
		}

		auto find(memblock block)
		{
			return std::find_if(_chunks.begin(), _chunks.end(), [&](auto& a){
				return a.owns(block);
			});
		}

	private:
		std::vector<child_allocator_t, allocator_wrapper<child_allocator_t, parent_allocator_t>> _chunks;
	};
}

#endif
