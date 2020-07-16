/*********************************************************************
* Allocator::Small_Object_Allocator                                  *
* Allocator::Small_Object_Allocator::Manager                         *
*                                                                    *
* Version: 1.0                                                       *
* Date:    16-07-202                                                 *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef SMALL_OBJECT_ALLOCATOR_H
#define SMALL_OBJECT_ALLOCATOR_H

#include <list>
#include "fixed_allocator.h"

namespace Allocator{

//####################################################################

class Small_Object_Allocator
{
	private:
		static Small_Object_Allocator* small_obj_allocator;
		std::list<Fixed_Allocator> allocators;
		unsigned int num_blocks;
		std::size_t max_object_size;
		Fixed_Allocator* last_allocation;
		Fixed_Allocator* last_deallocation;
		bool flushed;

		class Manager
		{
			private:
				Small_Object_Allocator* instance;
			public:
				Manager(Small_Object_Allocator* _instance)
				: instance(_instance){};
				~Manager(){
					instance->flush();
				}
		};

		Small_Object_Allocator(unsigned int _num_blocks, std::size_t _max_object_size);
		Small_Object_Allocator(const Small_Object_Allocator&);
		Small_Object_Allocator()=delete;
		Small_Object_Allocator& operator=(const Small_Object_Allocator&);
		~Small_Object_Allocator(){
			flush();
		}

	public:
		static Small_Object_Allocator& get_instance();
		static void init(unsigned int _num_blocks, std::size_t _max_object_size=64);
		void* allocate(std::size_t block_size);
		void deallocate(void* p, std::size_t block_size);
		void flush();
		template<typename T, typename... Args>
		T* alloc(Args&& ...args);
		template<typename T>
		void free(T* p);
		template<typename T>
		void free_all();
};

//======================================================================

inline void Small_Object_Allocator::init(unsigned int _num_blocks, std::size_t _max_object_size){
	if(small_obj_allocator){
		return;
	}
	small_obj_allocator=new Small_Object_Allocator(_num_blocks, _max_object_size);
	static Manager manager(small_obj_allocator);
}

//======================================================================

inline Small_Object_Allocator& Small_Object_Allocator::get_instance(){
	return *small_obj_allocator;
}

//======================================================================

template<typename T, typename... Args>
inline T* Small_Object_Allocator::alloc(Args&& ...args){
	return new(allocate(sizeof(T))) T(std::forward<Args>(args)...);
}

//======================================================================

template<typename T>
inline void Small_Object_Allocator::free(T* p){
	p->T::~T();
	deallocate(p, sizeof(T));
}

//======================================================================

template<typename T>
void Small_Object_Allocator::free_all(){
	if(flushed){
		return;
	}
	flushed=true;
	std::list<Fixed_Allocator>::iterator it=allocators.begin();
	do{
		it->free_all<T>();
	}while(++it!=allocators.end());
}

//####################################################################

}

#endif
