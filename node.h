/*********************************************************************
* Node class                                									*
* TReF struct                                                        *
*                                                           	      *
* Version: 1.0                                                       *
* Date:    09-11-2019                                                *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef NODE_H
#define NODE_H

#include<iostream>
#include<list>

//####################################################################

namespace DM{

enum class INDEX_TYPE : bool{UNIQUE=true, NO_UNIQUE=false};

bool operator!(INDEX_TYPE indx){
	if(indx==INDEX_TYPE::UNIQUE){
		return false;
	}
	return true;
}

//####################################################################

template<typename S>
struct TReF
{
	typedef typename std::remove_pointer<S>::type SS;
	
	SS* df;
	TReF(SS** fg)
		:df(*fg){};
	TReF(SS* gh)
		:df(gh){};
	TReF(SS gh)
		:df(&gh){};
	SS* operator()(){
		return df;
	}
};	

//####################################################################
//####################################################################

template<typename S>
struct DataContainer
{
	S data;

	DataContainer(const S& val)
		:data(val){};	

	DataContainer(S&& val)
		:data(std::move(val)){};	
};

//####################################################################
//####################################################################

template<typename S>
struct Node
{
	DataContainer<S>* data_cont=nullptr;
	std::list<S>* storage=nullptr;

	int height=0;
	Node<S>* left=nullptr;
	Node<S>* right=nullptr;
	Node<S>* top=nullptr;
	Node<S>* down=nullptr;
	Node<S>* up=nullptr;

	~Node(){	}

	Node(DataContainer<S>* tmp, bool setStorage)
	:data_cont(tmp){
		if(setStorage){
			storage=new std::list<S>(); 
		}
	}
	
	int diff(){
		int a=0;
		if(left){
			a=-1*(left->height+1);
		}
		if(right){
			a+=(right->height+1);
		}
		return a;
	}
	void bridge(Node<S>* tmp){
		if(top){
			if(top->right==this){
				top->right=tmp;
			}
			else{
				top->left=tmp;
			}
		}
		tmp->top=top;
		top=tmp;
	}
	inline int getLHeight()const{
		if(left){
			return left->height;
		}
		return 0;
	}
	inline int getRHeight()const{
		if(right){
			return right->height;
		}
		return 0;
	}
	inline void updateHeight(){
		if(left || right){
			height=std::max(getLHeight(), getRHeight())+1;	
		}
		else{
			height=0;
		}
	}
};

//####################################################################
//####################################################################


}

#endif
