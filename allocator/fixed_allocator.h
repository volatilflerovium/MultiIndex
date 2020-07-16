/*********************************************************************
* Allocator::Fixed_Allocator                                         *
*                                                                    *
* Version: 1.0                                                       *
* Date:    16-07-202                                                 *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef FIXED_ALLOCATOR_H
#define FIXED_ALLOCATOR_H

#include <cstring>
#include <list>
#include "chunk.h"

namespace Allocator
{

//####################################################################

class Fixed_Allocator
{
	private:
		std::list<Chunk> chunks;
		std::size_t block_size;
		std::list<Chunk>::iterator zt;
		unsigned int num_blocks;
		Chunk* last_allocated;
		Chunk* last_deallocated;
		bool* is_set;
		const unsigned int allocation_size;
		bool available;
		void pop_empty();
		void add_chunk();

	public:
		Fixed_Allocator(std::size_t _block_size,  unsigned int _num_blocks);
		Fixed_Allocator()=delete;
		~Fixed_Allocator();
		void* allocate();
		void deallocate(void* p);
		std::size_t size() const;
		void flush();

		template<typename T>
		void free_all();
		
		template<typename Func>
		void apply_on(Func cbk);
};

//======================================================================

inline Fixed_Allocator::~Fixed_Allocator(){
	delete[] is_set;
}

//======================================================================

inline std::size_t Fixed_Allocator::size() const{
	return block_size;
};

//======================================================================

inline void Fixed_Allocator::flush(){
	std::list<Chunk>::iterator it=chunks.begin();
	do{
		it->release();
	}while(++it!=chunks.end());
}

//======================================================================

template<typename T>
void Fixed_Allocator::free_all(){			
	unsigned char i=0;
	unsigned char* p;
	std::list<Chunk>::iterator it=chunks.begin();
	while(it!=chunks.end()){
		p=it->p_data;
		i=0;
		do{
			(reinterpret_cast<T*>(p))->T::~T();
			p+=block_size;
		}while(++i!=num_blocks);
		it->release();
		++it;
	}
}

//======================================================================

template<typename Func>
void Fixed_Allocator::apply_on(Func cbk){
	unsigned char y, j=0;
	std::list<Chunk>::iterator it=chunks.begin();
	Chunk* tmp_chunk;
	while(it!=chunks.end()){	
		std::memset(is_set, true, 1*num_blocks);
		tmp_chunk=&(*it);
		j=tmp_chunk->available_blocks;
		if(j>0){
			y=tmp_chunk->first_available_block;
			while(j-->0){
				is_set[(int)(y)]=false;
				y=*(tmp_chunk->p_data+block_size*y);
			}
		}
		unsigned char* p_data=static_cast<unsigned char*>(tmp_chunk->p_data);
		unsigned int i=0;
		do{
			if(is_set[i]){
				cbk(p_data+block_size*i);
			}
		}while(++i!=num_blocks);
		++it;
	}
}

//####################################################################

}

#endif
