#ifndef INCLGUARD_allocator_wrapper_hpp
#define INCLGUARD_allocator_wrapper_hpp

#include <cstddef>
#include <boost/align/align_up.hpp>
#include "memblock.hpp"

namespace alloc
{
	template <typename T, typename Allocator, std::size_t Alignment = alignof(T)>
	class allocator_wrapper
	{
	public:
		template<typename U>
		struct rebind
		{
			using other = allocator_wrapper<U, Allocator, Alignment>;
		};

	public:
		using value_type = T;
		using propagate_on_container_copy_assignment = std::true_type;
		using propagate_on_container_move_assignment = std::true_type;
		using propagate_on_container_swap = std::true_type;

		static constexpr std::size_t alignment = Alignment;

		allocator_wrapper(Allocator* allocator) noexcept
			: _allocator(allocator)
		{ }

		template<typename U>
		allocator_wrapper(allocator_wrapper<U, Allocator, Alignment> const& other) noexcept
			: _allocator{other._allocator}
		{ }

		template<typename U>
		allocator_wrapper& operator= (allocator_wrapper<U, Allocator, Alignment> const& other) noexcept
		{
			_allocator = other._allocator;
			return *this;
		}

		T* allocate(std::size_t n)
		{
			constexpr std::size_t actual_allignment = std::max(sizeof(memblock::size_t), alignment);
			memblock block = _allocator->allocate(actual_allignment + sizeof(T) * n, actual_allignment);
			if(block.ptr)
			{
				*reinterpret_cast<memblock::size_t*>(block.ptr) = block.size;
				return reinterpret_cast<T*>(reinterpret_cast<char*>(block.ptr) + actual_allignment);
			}
			else
				throw std::bad_alloc{};
		}

		void deallocate(T* ptr, std::size_t)
		{
			constexpr std::size_t actual_allignment = std::max(sizeof(memblock::size_t), alignment);

			memblock block{
				reinterpret_cast<char*>(ptr) - actual_allignment,
				*reinterpret_cast<memblock::size_t*>(reinterpret_cast<char*>(ptr) - actual_allignment)};

			assert(_allocator->owns(block));
			_allocator->deallocate(block);
		}

		template <typename T_, typename U, typename Allocator_>
		friend bool operator==(const allocator_wrapper<T_, Allocator_>& lhs, const allocator_wrapper<U, Allocator_>& rhs);

		template<typename T_, typename U, typename Allocator_>
		friend bool operator!=(const allocator_wrapper<T_, Allocator_>& lhs, const allocator_wrapper<U, Allocator_>& rhs);

	private:
		Allocator* _allocator;
	};

	template <typename T_, typename U, typename Allocator_>
	bool operator==(const allocator_wrapper<T_, Allocator_>& lhs, const allocator_wrapper<U, Allocator_>& rhs)
	{
		return lhs._allocator == rhs._allocator;
	}

	template <typename T_, typename U, typename Allocator_>
	bool operator!=(const allocator_wrapper<T_, Allocator_>& lhs, const allocator_wrapper<U, Allocator_>& rhs)
	{
		return !(lhs == rhs);
	}
} // namepsace alloc

#endif
