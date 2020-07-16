
#include "fixed_allocator.h"

using namespace Allocator;

//======================================================================

Fixed_Allocator::Fixed_Allocator(std::size_t _block_size, unsigned int _num_blocks)
	:block_size(_block_size), num_blocks(_num_blocks), 
	last_allocated(nullptr),
	last_deallocated(nullptr),
	is_set(new bool[num_blocks]),
	allocation_size(block_size*num_blocks),
	available(false)
{
	add_chunk();
	last_deallocated=last_allocated;
	zt=chunks.begin();
}

//======================================================================

void Fixed_Allocator::add_chunk(){
	chunks.emplace_back();
	last_allocated=&chunks.back();
	last_allocated->init(block_size, num_blocks);
}

//======================================================================

void* Fixed_Allocator::allocate(){
	if(last_allocated->available_blocks==0){
		if(!available){
			add_chunk();
			return last_allocated->allocate(block_size);
		}

		std::list<Chunk>::iterator ck=chunks.begin();
		for(;;++ck){
			if(ck==chunks.end()){
				available=false;
				add_chunk();
				break;
			}
			if(ck->available_blocks>0){
				last_allocated=&(*ck);
				break;
			}
		}
	}
	return last_allocated->allocate(block_size);
}

//======================================================================

void Fixed_Allocator::pop_empty(){
	last_allocated=&(*last_deallocated);
	available=true;
}

//======================================================================

void Fixed_Allocator::deallocate(void* p){
	unsigned char* to_release=static_cast<unsigned char*>(p);

	if(last_deallocated->in_chunk(to_release, allocation_size)){
		last_deallocated->deallocate(to_release, block_size);
		available=true;
		if(last_deallocated->available_blocks==num_blocks){
			std::list<Chunk>::iterator it=zt;
			if(++zt==chunks.end()){
				zt=it;
				if(zt==chunks.begin()){
					return;	
				}
				--zt;
			}
			last_deallocated=&(*zt);
			available=false;
			chunks.erase(it);
		}
		last_allocated=last_deallocated;		
		return;
	}

	Chunk* ck;
	std::list<Chunk>::iterator it=zt;
	while(it!=chunks.end()){
		ck=&(*it);
		if(ck->in_chunk(to_release, allocation_size)){
			ck->deallocate(to_release, block_size);
			available=true;
			if(ck->available_blocks==num_blocks){
				zt=it;
				ck=&(*(--zt));
				chunks.erase(it);
				available=false;
			}
			last_deallocated=ck;
			last_allocated=ck;
			return;
		}
		++it;
	}
		
	zt=chunks.begin();
	while(zt!=chunks.end()){
		ck=&(*zt);
		if(ck->in_chunk(to_release, allocation_size)){
			ck->deallocate(to_release, block_size);
			available=true;
			if(ck->available_blocks==num_blocks){
				it=zt;
				ck=&(*(++zt));
				chunks.erase(it);
				available=false;
			}
			last_deallocated=ck;
			last_allocated=ck;
			return;
		}
		--zt;
	}
}

//======================================================================
