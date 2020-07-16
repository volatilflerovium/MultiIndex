
#include "small_object_allocator.h"

using namespace Allocator;

//======================================================================

Small_Object_Allocator* Small_Object_Allocator::small_obj_allocator=nullptr;

//======================================================================

Small_Object_Allocator::Small_Object_Allocator(
			unsigned int _num_blocks, 
			std::size_t _max_object_size)
	: num_blocks(_num_blocks), 
	max_object_size(_max_object_size),
	last_allocation(nullptr),
	last_deallocation(nullptr),
	flushed(false)
{
	allocators.push_back(Fixed_Allocator(0, 0));
	last_allocation=&(allocators.back());
	last_deallocation=last_allocation;
}

//======================================================================

void* Small_Object_Allocator::allocate(std::size_t block_size){
	if(last_allocation->size()==block_size){
		return last_allocation->allocate();
	}
	
	if(block_size>max_object_size){
		return ::operator new(block_size);
	}

	std::list<Fixed_Allocator>::iterator it=allocators.begin();

 	for(;;++it){
		if(it==allocators.end()){
			allocators.push_back(Fixed_Allocator(block_size, num_blocks));
			last_allocation=&(allocators.back());
			break;
		}
		else if(it->size()>block_size){
			allocators.insert(it, Fixed_Allocator(block_size, num_blocks));
			last_allocation=&(*it);	
			break;
		}
		else if(it->size()==block_size){
			last_allocation=&(*it);	
			break;
		}
	}
	return last_allocation->allocate();
}

//======================================================================

void Small_Object_Allocator::deallocate(void* p, std::size_t block_size){
	if(last_deallocation->size()==block_size){
		last_deallocation->deallocate(p);
		return;
	}

	if(block_size>max_object_size){
		::operator delete(p);
	}
	
	std::list<Fixed_Allocator>::iterator it=allocators.begin();
	while(it!=allocators.end()){
		if(it->size()==block_size){
			last_deallocation=&(*it);
			last_deallocation->deallocate(p);
			break;
		}
		++it;
	}
}

//======================================================================

void Small_Object_Allocator::flush(){
	if(flushed){
		return;
	}
	flushed=true;	
	std::list<Fixed_Allocator>::iterator it=allocators.begin();
	do{
		it->flush();
	}while(++it!=allocators.end());
}

//======================================================================
