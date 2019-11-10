/*********************************************************************
* DM::DBTable class                               						*
*                                                                    *
* Version: 1.0                                                       *
* Date:    09-11-2019                                                *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef DB_TABLE_H
#define DB_TABLE_H

#include<iostream>
#include<list>
#include<vector>
#include<map>

#include "index.h"

using namespace std;
using namespace DM;

//####################################################################

template<typename T>
class DBTable
{
	typedef typename std::remove_pointer<T>::type TT;

	template<typename S>
	struct Wp
	{
		typedef typename std::remove_pointer<S>::type SS;
		S data;
		Wp(const S& _data)
			:data(_data){};	
		Wp(S&& _data)
			:data(std::move(_data)){};		
		S* getThis(){
			return &data;
		}
	};

	private:
		std::list<Wp<T>> Records;
		std::vector<std::string> indexesNames;
		std::map<std::string, Index<TT*>*> Indexes;
		bool loadData(std::string indexName);
		bool removeResource;
		bool inserting(TT* data);
		bool indexExist(std::string indexName);

	public:
		~DBTable();
		DBTable();
		template<typename... Args>
		bool registerIndex(std::string indexName, INDEX_TYPE isUnique, Args... args);

		void remove(const T& data);

		void insert(TT&& data);
		void insert(TT* data);
	
		Query<T> mkQuery();
	
		void print(std::string usingIndexName);
	
		friend class Query<T>;
};

//====================================================================

template<typename T>
DBTable<T>::~DBTable(){
	for(auto pos=Indexes.begin(); pos!=Indexes.end(); ++pos){
		delete Indexes[pos->first];
	}
	if(removeResource){
		typename std::list<Wp<T>>::iterator it=Records.begin();
		while(it!=Records.end()){
			TReF<TT> tRef((*it).getThis());
			delete tRef();
			++it;
		}
	}
}

//====================================================================

template<typename T>
bool DBTable<T>::indexExist(std::string indexName){
	if(Indexes.find(indexName)==Indexes.end()){ 
		return false;
	}
	return true;
}

//====================================================================

template<typename T>
DBTable<T>::DBTable()
	:removeResource(true){
		removeResource=std::is_same<T,TT*>::value;
}

//====================================================================

template<typename T>
bool DBTable<T>::loadData(std::string indexName){
	if(Records.size()>0){
		typename std::list<DBTable<T>::Wp<T>>::iterator it=Records.begin();
		while(it!=Records.end()){
			try{
				TReF<TT> tRef((*it).getThis());
 				Indexes[indexName]->insert(tRef());
			}
			catch(const char* msg){
				std::cout<<msg<<std::endl;
				return false;
			}
			++it;
		}
	}
	return true;
}

//====================================================================

template<typename T>
void DBTable<T>::print(std::string usingIndexName){
	Indexes[usingIndexName]->print();
}

//====================================================================

template<typename T>
template<typename... Args>
bool DBTable<T>::registerIndex(std::string name, INDEX_TYPE isUnique, Args... args){
	if(!indexExist(name)){
		Indexes[name]=new Index<TT*>(isUnique, args...);
		Indexes[name]->noRemoveData();
		if(!loadData(name)){
			delete Indexes[name];
			Indexes.erase(name);
			return false;
		}
		indexesNames.push_back(name);
		return true;
	}
	throw "Index already exists.";
	return false;
}

//====================================================================

template<typename T>
void DBTable<T>::insert(TT* data){
	Records.push_back(Wp<T>(data));
	typename std::list<DBTable<T>::Wp<T>>::iterator it=Records.end();
	--it;
	TReF<TT> tRef((*it).getThis());
	TT* tg=tRef();
	if(!inserting(tg) && removeResource){
		delete tg;
	}
}
//====================================================================

template<typename T>
void DBTable<T>::insert(TT&& data){
	Records.push_back(Wp<T>(std::move(data)));
	typename std::list<DBTable<T>::Wp<T>>::iterator it=Records.end();
	--it;
	TReF<TT> tRef((*it).getThis());
	TT* tg=tRef();
	if(!inserting(tg) && removeResource){
		delete tg;
	}
}

//====================================================================
//*
template<typename T>
bool DBTable<T>::inserting(TT* data){
	bool a=true;
	std::vector<std::string> names;
	for(auto pos=Indexes.begin(); pos!=Indexes.end(); ++pos){
		names.push_back(pos->first);
		try{
			pos->second->insert(data);
		}
		catch(const char* msg){
			std::cout<<msg<<std::endl;
			for(int i=0; i<names.size(); i++){
				Indexes[names[i]]->remove(data);
			}
			Records.pop_back();
			a=false;
			break;
		}
	}
	return a;
}

//====================================================================

template<typename T>
void DBTable<T>::remove(const T& data){
	for(auto pos=Indexes.begin(); pos!=Indexes.end(); ++pos){
		pos->second->remove(&data);
	}
}

//====================================================================

template<typename T>
Query<T> DBTable<T>::mkQuery(){
	return Query<T>(this);
}

//####################################################################

#endif

