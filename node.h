/*********************************************************************
* DM::Node class                            									*
*                                                                    *
* Version: 1.1                                                       *
* Date:    16-07-202                                                 *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef NODE_H
#define NODE_H

#include<list>

#include "allocator/custom_allocator.h"

using namespace Allocator;
//####################################################################

namespace DM{

//####################################################################

template<typename S>
struct Node
{
	S data;
	std::list<S, Custom_Allocator<S>>* storage=nullptr;

	int height=0;
	Node<S>* left=nullptr;
	Node<S>* right=nullptr;
	Node<S>* top=nullptr;
	Node<S>* down=nullptr;
	Node<S>* up=nullptr;

	~Node(){
		delete storage;
	}

	Node(S tmp, bool setStorage)
	:data(tmp){
		if(setStorage){
			storage=new std::list<S, Custom_Allocator<S>>(); 
		}
	}

	int diff();

	void bridge(Node<S>* tmp);

	void forkNode(Node<S>* blankNode);

	int getLHeight()const{
		if(left){
			return left->height;
		}
		return 0;
	}
	
	int getRHeight()const{
		if(right){
			return right->height;
		}
		return 0;
	}

	void updateHeight(){
		if(left || right){
			height=std::max(getLHeight(), getRHeight())+1;	
		}
		else{
			height=0;
		}
	}



};

template<typename S>
int Node<S>::diff(){
	int a=0;
	if(left){
		a=-1*(left->height+1);
	}
	if(right){
		a+=(right->height+1);
	}
	return a;
}

template<typename S>
void Node<S>::forkNode(Node<S>* blankNode){
	blankNode->storage=storage;
	storage=nullptr;
	blankNode->up=this;
	down=blankNode;
}

template<typename S>
void Node<S>::bridge(Node<S>* tmp){
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

//####################################################################

}

#endif
