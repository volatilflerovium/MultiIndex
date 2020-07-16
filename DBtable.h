/*********************************************************************
* DM::DBTable class                                						*
*                                                                    *
* Version: 1.1                                                       *
* Date:    16-7-202                                                  *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef DB_TABLE_H
#define DB_TABLE_H

#define DEBUG true

#include <vector>

#include "index.h"
#include "container.h"

using namespace std;
namespace DM{

//####################################################################

template<typename T>
class DBTable
{
	typedef typename std::remove_pointer<T>::type TT;

	private:
		Container<T> Records;
		std::vector<std::string> indexesNames;
		std::vector<Index<T>*> Indexes;
		int indexExist(std::string indexName);

	public:
		~DBTable();
		DBTable();
		template<typename... Args>
		bool registerIndex(std::string indexName, INDEX_TYPE isUnique, Args... args);

		void remove(const T& data);

		void clear();

		template<typename... Args>
		void insert(Args&& ...args);

		Query<T> mkQuery();

		void print(std::string usingIndexName);

		friend class Query<T>;
};

//====================================================================

template<typename T>
void DBTable<T>::clear(){
	Records.clean();
}

//====================================================================

template<typename T>
DBTable<T>::~DBTable(){
	for(int i=0; i<Indexes.size(); i++){
		delete Indexes[i];
	}
	if(DEBUG && std::is_same<T,TT*>::value){
		std::cout<<"Use the method clear() to delete pointers\n";
	}
}

//====================================================================

template<typename T>
DBTable<T>::DBTable()
	:Records(255)
{
	Index<T>::set_allocator(&Records);
}

//====================================================================

template<typename T>
int DBTable<T>::indexExist(std::string index_name){
	for(int i=0; i<indexesNames.size(); i++){
		if(index_name==indexesNames[i]){
			return i;
		}
	}
	return -1;
}

//====================================================================

template<typename T>
void DBTable<T>::print(std::string usingIndexName){
	int n=indexExist(usingIndexName);
	if(n>-1){
		Indexes[n]->print();
	}
}

//====================================================================

template<typename T>
template<typename... Args>
bool DBTable<T>::registerIndex(std::string name, INDEX_TYPE isUnique, Args... args){
	if(indexExist(name)==-1){
		Indexes.reserve(Indexes.size()+1);
		Index<T>* index=new Index<T>(isUnique, args...);
		Indexes.push_back(index);

		if(!index->load_data()){
			delete index;
			Indexes.pop_back();
			return false;
		}
		Indexes.reserve(Indexes.size()+1);
		indexesNames.push_back(name);
		return true;
	}
	throw "Index already exists.";	
	return false;
}

//====================================================================

template<typename T>
template<typename... Args>
void DBTable<T>::insert(Args&& ...args){
	TT* ptr=Records.insert(std::forward<Args>(args)...);
	int a=0;
	int last=0;	
	for(;last<Indexes.size(); ++last){
		try{
			Indexes[last]->insert(ptr);
		}
		catch(const char* msg){
			std::cout<<msg<<std::endl;
			a++;
		}
	}
	if(a==Indexes.size()){
		Records.deallocate_node(ptr);
	}
}

//====================================================================

template<typename T>
void DBTable<T>::remove(const T& data){
	for(int i=0; i<Indexes.size(); i++){
		Indexes[i]->remove(&data);
	}
}

//====================================================================

template<typename T>
Query<T> DBTable<T>::mkQuery(){
	return Query<T>(this);
}

//####################################################################

}

#endif
