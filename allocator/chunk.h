/*********************************************************************
* Allocator::Chunk structure                                         *
*                                                                    *
* Version: 1.0                                                       *
* Date:    16-07-202                                                 *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef CHUNK_H
#define CHUNK_H

#include <cstdlib>

namespace Allocator{

struct Chunk{
	unsigned char* p_data;
	unsigned char first_available_block;
	unsigned char available_blocks;

	void init(std::size_t block_size, unsigned char blocks);
	void release();
	void* allocate(std::size_t block_size);
	void deallocate(unsigned char* to_release, std::size_t block_size);
	bool in_chunk(unsigned char* p, unsigned int r);
	int get_position(unsigned char* p);
};
/*
Chunk does not define constructors, destructors, or assignment
operator. Defining proper copy semantics at this level hurts efficiency 
at upper levels, where we store Chunk objects in a list.
*/
//======================================================================

inline bool Chunk::in_chunk(unsigned char* p, unsigned int r){
	if(p_data<=p && p<=p_data+r){
		return true;
	}
	return false;
}

//======================================================================

// --- end namespace
}



#endif
