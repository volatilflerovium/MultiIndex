/*********************************************************************
* DM::Query class                               							*
*                                                                    *
* Version: 1.0                                                       *
* Date:    09-11-2019                                                *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef QUERY_H
#define QUERY_H

#include<iostream>
#include<list>
#include<vector>
#include<map>

#include "index.h"
#include "key_filter.h"

using namespace std;
namespace DM{

const int MAXK=40;


//####################################################################
//####################################################################

template<typename T>
class Query
{
	typedef typename std::remove_pointer<T>::type TT;
	typedef DBTable<T> DBTablex;

	private:
		RuleManager<TT>* queryKey;
		
		Index<TT*>* mIndex;
		DBTable<T>* MIndex;
		std::map<std::string, int*> rank;
		std::vector<TT*> results;
		bool ready;
		void clear();
		
		void setup_query();

		std::vector<TT*> scan(bool usingKey);
		template<typename S>
		void insertRule(S TT::* field, Rule<TT>* rule);

	public:

		~Query();

		Query(DBTable<T>* multiIndex);

		template<typename S>
		Query<T>& selectWhere(S TT::* field, const S& p, COMPARE lsGt);

		template<typename S>
		Query<T>& selectWhere(S TT::* field, std::pair<S, S> vpr);
		
		template<typename S>
		Query<T>& selectWhere(S TT::* field, std::vector<S> vp);

		template<typename S>
		Query<T>& selectWhere(S TT::* field, std::initializer_list<S> lvp);

		std::vector<TT*> getResults();
		
		template<typename S>
		std::vector<S> getResults(S TT::* field);

		template<class... Args>
		T getUnique(const std::string& indexName, Args... args);

		template<typename S>
		std::map<S, TT*> mapON(S TT::* field);
};

//====================================================================

template<typename T>
Query<T>::~Query(){
	if(queryKey){
		delete queryKey;
	}

	for(int i=0; i<MIndex->indexesNames.size(); i++){
		delete[] rank[MIndex->indexesNames[i]];
	}
}

//====================================================================

template<typename T>
Query<T>::Query(DBTable<T>* multiIndex)
	: MIndex(multiIndex), ready(false), 
	 queryKey(nullptr)
{
	for(int i=0; i<MIndex->indexesNames.size(); i++){
		rank[MIndex->indexesNames[i]]=new int[MAXK];
	}		
}

//====================================================================

template<typename T>
std::vector<typename Query<T>::TT*> Query<T>::scan(bool usingKey){
	std::vector<TT*> search_results;
	if(usingKey){
		queryKey->getResult(search_results, mIndex->Root);
	}
	else{
		queryKey->getData(search_results, mIndex->Root);
	}
	return search_results;
}

//====================================================================

template<typename T>
void Query<T>::setup_query(){
	if(!ready){
		ready=true;
		queryKey=new RuleManager<TT>;

		for(int i=0; i<MIndex->indexesNames.size(); i++){
			for(int j=0; j<MAXK; j++){
				rank[MIndex->indexesNames[i]][j]=-1;
			}
		}
	}
}

//====================================================================

template<typename T>
template<typename S>
void Query<T>::insertRule(S TT::* field, Rule<TT>* rule){
	int k=0, j=-1;
	bool a=true;

	for(int i=0; i<MIndex->indexesNames.size(); i++){
		k=(MIndex->Indexes)[MIndex->indexesNames[i]]->inKey(field);
		if(k>-1){
			if(a){
				j=queryKey->insertRule(rule);
				a=false;
			}
			rank[MIndex->indexesNames[i]][k]=j;
		}
	}

	if(a){
		queryKey->insertRule(rule, true);
	}
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectWhere(S TT::* field, const S& p, COMPARE lsGt){
	setup_query();
	insertRule(field, queryKey->mkRule(field, p, lsGt));

	return *this;
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectWhere(S TT::* field, std::initializer_list<S> vp){
	setup_query();
	insertRule(field, queryKey->mkRule(field, vp));

	return *this;
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectWhere(S TT::* field, std::vector<S> vp){
	setup_query();
	insertRule(field, queryKey->mkRule(field, vp));

	return *this;
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectWhere(S TT::* field, pair<S, S> vp){
	setup_query();
	insertRule(field, queryKey->mkRule(field, vp));

	return *this;
}

//====================================================================

template<typename T>
void Query<T>::clear(){
	results.clear();
	delete queryKey;
	queryKey=nullptr;

	ready=false;

	for(int i=0; i<MIndex->indexesNames.size(); i++){
		for(int j=0; j<MAXK; j++){
			rank[MIndex->indexesNames[i]][j]=-1;
		}
	}
}

//====================================================================

template<typename T>
std::vector<typename Query<T>::TT*> Query<T>::getResults(){
	if(!ready){
		return std::vector<TT*>();
	}

	int k=0,mx=-1;
	for(int i=0; i<MIndex->indexesNames.size(); i++){
		for(int j=0; j<MAXK; j++){
			if(rank[MIndex->indexesNames[i]][j]>-1){
				if(mx<j){
					mx=j;
					k=i;
				}
			}
			else{
				break;
			}
		}
	}

	bool usingKey=true;
	if(mx<0){
		//Do full search
		usingKey=false;
	}
	queryKey->sortKey(rank[MIndex->indexesNames[k]], mx+1);

	mIndex=MIndex->Indexes[MIndex->indexesNames[k]];
	std::vector<TT*> results2(scan(usingKey));
	clear();
	
	return results2;
}

//====================================================================

template<typename T>
template<typename S>
std::vector<S> Query<T>::getResults(S TT::* field){
	std::vector<TT*> result_data(getResults());	
	std::vector<S> results2;
	for(int i=0; i<result_data.size(); i++){
		results2.push_back(result_data[i]->*field);
	}

	return results2;
}	

//====================================================================

template<typename T>
template<typename S>
std::map<S,typename Query<T>::TT*> Query<T>::mapON(S TT::* field){
	std::map<S, TT*> results2;
	for(auto t : results){
		results2[t->*field]=t;
	}
	return results2;
}

//====================================================================

template<typename T>
template<class... Args>
T Query<T>::getUnique(const std::string& indexName, Args... args){
	if(!MIndex->indexExist(indexName)){
		throw "Index name does not exist.";
	}
	return (MIndex->Indexes[indexName])->getUnique(args...);
}


//####################################################################

}


#endif

