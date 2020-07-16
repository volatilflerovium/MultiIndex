/*********************************************************************
* DM::Container class                                                *
*                                                                    *
* Version: 1.1                                                       *
* Date:    16-07-202                                                 *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef CONTAINER_H
#define CONTAINER_H
#include <cstring>

#include "allocator/fixed_allocator.h"
#include "node.h"
#include "utility.h"

using namespace Allocator;

namespace DM
{

//####################################################################

template<typename T>
class Container
{
	typedef typename std::remove_pointer<T>::type No_Pointer;

	private:
		Fixed_Allocator data_storage;
		Fixed_Allocator node_storage;
		bool* is_set;
		unsigned char num_blocks;

		Container()=delete;
		Container& operator=(const Container&);
		void remove_content(Int2Type<true>);
		void remove_content(Int2Type<false>);

		void delete_pointers(Int2Type<false>);
		void delete_pointers(Int2Type<true>);

	public:

		Container(unsigned char _num_blocks)
			: data_storage(sizeof(T), _num_blocks),
			node_storage(sizeof(Node<No_Pointer*>), _num_blocks),
			num_blocks(_num_blocks),
			is_set(new bool[_num_blocks])
		{};

		~Container();

		template<typename... Args>
		No_Pointer* insert(Args&& ...args){
			return new(data_storage.allocate()) T(std::forward<Args>(args)...);
		};

		No_Pointer* insert(No_Pointer* ptr){ 
			new(data_storage.allocate()) No_Pointer*(ptr); 
			return ptr;
		}

		void free(T* p){
			p->T::~T();
			data_storage.deallocate(p);
		};
		
		void clean();

		Node<No_Pointer*>* new_node(No_Pointer* ptr, bool a){
			return new(node_storage.allocate()) Node<No_Pointer*>(ptr, a);
		}

		void free_node(Node<No_Pointer*>* node){
			node->Node<No_Pointer*>::~Node<No_Pointer*>();
		}

		void deallocate_node(void* p){
			node_storage.deallocate(p);
		}
		
		template<typename Func>
		void apply_on(Func cbk);
		
		No_Pointer* get_data(unsigned char* p_data, Int2Type<true>);
		No_Pointer* get_data(unsigned char* p_data, Int2Type<false>);
};

//======================================================================

template<typename T>
void Container<T>::remove_content(Int2Type<true>){
	data_storage.flush();
	node_storage.flush();
}

//======================================================================

template<typename T>
void Container<T>::remove_content(Int2Type<false>){
	data_storage.apply_on([](unsigned char* ptr)->void{
		(reinterpret_cast<T*>(ptr))->T::~T();
	});
	data_storage.flush();
	node_storage.flush();
}

//======================================================================

template<typename T>
Container<T>::~Container(){
	delete[] is_set;
	remove_content(Int2Type<Is_Pointer<T>::is_pointer>());
}

//======================================================================

template<typename T>
inline typename Container<T>::No_Pointer* Container<T>::get_data(unsigned char* p_data, Int2Type<false>){
	return reinterpret_cast<No_Pointer*>(p_data);
}

//======================================================================

template<typename T>
inline typename Container<T>::No_Pointer* Container<T>::get_data(unsigned char* p_data, Int2Type<true>){
	return *(reinterpret_cast<No_Pointer**>(p_data));
}

//======================================================================

template<typename T>
template<typename Func>
void Container<T>::apply_on(Func cbk){
	data_storage.apply_on(cbk);
}

//======================================================================

template<typename T>
void Container<T>::delete_pointers(Int2Type<false>){
	data_storage.apply_on([](unsigned char* ptr)->void{
		delete *(reinterpret_cast<No_Pointer**>(ptr));
	});
}

//======================================================================

template<typename T>
void Container<T>::delete_pointers(Int2Type<true>){
}

//======================================================================

template<typename T>
void Container<T>::clean(){
	delete_pointers(Int2Type<!Is_Pointer<T>::is_pointer>());
}

//######################################################################

}// end of namespace DM

#endif
