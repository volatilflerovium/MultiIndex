/*********************************************************************
* Allocator::Custom_Allocator                                        *
*                                                                    *
* Version: 1.0                                                       *
* Date:    16-07-202                                                 *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef CUSTOM_ALLOCATOR_H
#define CUSTOM_ALLOCATOR_H

#include "small_object_allocator.h"

using namespace std;

namespace Allocator{

//======================================================================

template <class T>
class Custom_Allocator
{
	public:
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;

		Small_Object_Allocator* mallocator;

		Custom_Allocator() {
			Small_Object_Allocator::init(255, 64);
			mallocator=&(Small_Object_Allocator::get_instance());
		}

		Custom_Allocator(const Custom_Allocator&)
		{}

		pointer allocate(size_type n, const void * = 0) {
			return (T*) mallocator->allocate(n * sizeof(T));
		}
  
		void	deallocate(void* p, size_type sz) {
			if (p) {
				mallocator->deallocate(p, sz);
			} 
		}

		pointer address(reference x) const { 
			return &x; 
		}

		const_pointer address(const_reference x) const { 
			return &x; 
		}

		Custom_Allocator<T>& operator=(const Custom_Allocator&){ 
			return *this; 
		}

		void construct(pointer p, const T& val){ 
			new ((T*) p) T(val); 
		}

		template<typename... Args> 
		void construct(pointer p, Args&&... args){
			new(p) T(std::forward<Args>(args)...);
		}

		void destroy(pointer p){ 
			p->~T(); 
		}
		
		size_type max_size() const { 
			return size_t(-1); 
		}
		
		template <class U>
		struct rebind 
		{ 
			typedef Custom_Allocator<U> other; 
		};

		template <class U>
		Custom_Allocator(const Custom_Allocator<U>&) 
		{}
		
		template <class U>
		Custom_Allocator& operator=(const Custom_Allocator<U>&){ 
			return *this; 
		}
};

//======================================================================
}

#endif
