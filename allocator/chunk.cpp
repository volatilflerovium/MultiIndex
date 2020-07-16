#include <cassert>
#include <cmath>

#include "chunk.h"

using namespace Allocator;

//======================================================================

void Chunk::init(std::size_t block_size, unsigned char blocks){
	p_data=new unsigned char[block_size*(blocks)];
	first_available_block=0;
	available_blocks=blocks;
	unsigned char i=0;
	unsigned char* p=p_data;
	while(i!=blocks){
		p[0]=++i;
		p+=block_size;
	}
}

//======================================================================

void Chunk::release(){
	delete[] p_data;
	available_blocks=0;
}

//======================================================================

void* Chunk::allocate(std::size_t block_size){
	unsigned char* result=p_data+first_available_block*block_size;

	first_available_block=*result;
	--available_blocks;
	return result;
}

//======================================================================

void Chunk::deallocate(unsigned char* to_release, std::size_t block_size){
	//assert(to_release>=p_data);

	//alligment check
	assert((to_release-p_data)%block_size==0);	
	*to_release=first_available_block;
	
	first_available_block=static_cast<unsigned char>((to_release-p_data)/block_size);

	assert(first_available_block*block_size==(to_release-p_data));
	++available_blocks;
}

//======================================================================
