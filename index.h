/*********************************************************************
* Index class                                								*
*                                                                    *
* Version: 1.0                                                       *
* Date:    09-11-2019                                                *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef INDEX_H
#define INDEX_H

#include<iostream>
#include<list>

#include "key.h"
#include "node.h"

//####################################################################

namespace DM{


//####################################################################

template<typename T>
class Query;

template<typename T>
class Index
{
	typedef typename std::remove_pointer<T>::type TT;

	private:
		
		enum LR{LEFT, RIGHT};
		Node<T>* Node<T>::* branch[2]={&Node<T>::left, &Node<T>::right};
		INDEX_TYPE isUnique;
		bool deleteResource;
		KeyCompound<TT> key;

		void balance(Node<T>* node, int diff);

		int insert(Node<T>* &, DataContainer<T>*, int);

		template<typename TA>
		Node<T>* tFind(const TA& value, int mx=0);

		bool remove(Node<T>* node);

		void deleting(Node<T>* node, bool recursive=true);

		void dataDeletion(Node<T>* node);
		
	public:
		Node<T>* Root;
		~Index();

		template<typename S, typename... Args>
		Index(INDEX_TYPE unique, S TT::* s, Args... args);

		void print(Node<T>* root=nullptr)const;

		void noRemoveData();

		void insert(const T& value);

		void insert(T&& value);

		bool remove(const T& value);

		template<typename S>
		T findUnique(S value);

		template<class... Args>
		T getUnique(Args... args);		

		TT* first();
		TT* last();

		template<typename S>
		int inKey(S TT::* ptr);

		int getKeySize()const;
		
	friend class Query<T>;
};

//====================================================================

template<typename T>
void Index<T>::dataDeletion(Node<T>* node){
	if(deleteResource && node->data_cont){
		TReF<TT> tRef(node->data_cont->data);
		if(std::is_same<T,TT*>::value){
			delete tRef();
			delete node->data_cont;
			node->data_cont=nullptr;
			if(!isUnique){
				auto it=node->storage->begin();
				while(it!=node->storage->end()){
					delete *it;
					++it;
				}
				delete node->storage;
			}
		}
	}
	delete node;
}

//====================================================================

template<typename T>
void Index<T>::deleting(Node<T>* node, bool recursive){

	if(node){
		Node<T>* tmp=node->right;
		Node<T>* tmp2=node->left;
		Node<T>* tmp3=node->down;

		dataDeletion(node);

		if(recursive){
			deleting(tmp);
			deleting(tmp2);
			deleting(tmp3);
		}
	}
}
//====================================================================

template<typename T>
Index<T>::~Index(){
	deleting(Root);
}

//====================================================================

template<typename T>
void Index<T>::noRemoveData(){
	deleteResource=false;
}

//====================================================================

template<typename T>
template<typename S, typename... Args>
Index<T>::Index(INDEX_TYPE _unique, S TT::* s, Args... args)
	:Root(nullptr), isUnique(_unique), 
		deleteResource(false){
	if(std::is_same<T,TT*>::value){
		deleteResource=true;
	}
	
	mkKey(key, s, args...);
}

//====================================================================

template<typename T>
void Index<T>::print(Node<T>* root)const{
	std::list<Node<T>*> NL; 
	std::list<Node<T>*> SBT; 
	if(!root){
		NL.push_back(Root);
	}
	else{
		NL.push_back(root);
	}

	int s=0, height=0;
	while(NL.size()){
		auto it=NL.begin();
		s=NL.size();
		for(int i=0; i<s; i++){
			if(*it){
				std::cout<<*((*it)->data_cont->data)<<" : ";
				NL.push_back((*it)->left);
				NL.push_back((*it)->right);
				if((*it)->down){
					SBT.push_back((*it)->down);
				}
			}
			else{
				std::cout<<"NULL : ";
			}
			++it;
			NL.pop_front();
		}
		height++;
		std::cout<<"\n-----------------------------------------------"<<std::endl;
	}
	std::cout<<"\n+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +"<<std::endl;
	
	auto sit=SBT.begin();
	s=SBT.size();
	//std::cout<<s<<"<< size"<<std::endl;
	for(int i=0; i<s; i++){
		print(*sit);
		++sit;
		SBT.pop_front();
	}
}

//====================================================================

template<typename T>
void Index<T>::balance(Node<T>* node, int diff){
	int lr, clr, ft;
	if(diff>0){
		lr=LR::LEFT;
		clr=LR::RIGHT;
		ft=1;
	}
	else if(diff<0){
		lr=LR::RIGHT;
		clr=LR::LEFT;
		ft=-1;
	}
	
	Node<T>* tmp=node->*branch[clr];
	Node<T>* tmp2=tmp->*branch[lr];

	bool isRoot=(Root==node);
	bool isRelRoot=(node->top==nullptr);

	//Node<T>* tmpDown=nullptr;
	Node<T>* tmpUp=nullptr;
	if(isRelRoot){
		if(node->up){
			tmpUp=node->up;
			tmpUp->down=nullptr;
			node->up=nullptr;
		}
	}

	if(!tmp2){
		node->bridge(tmp);
		node->*branch[clr]=nullptr;
		tmp->*branch[lr]=node;	
		node->height-=2;
		tmp2=tmp;
	}
	else{
		int diff=tmp->diff();

		if(!(tmp->*branch[clr])){ 
			node->bridge(tmp2);
			
			tmp2->*branch[lr]=node;
			tmp2->*branch[clr]=tmp;
			
			tmp->*branch[lr]=nullptr;
			node->*branch[clr]=nullptr;
			tmp->top=tmp2;
			node->height=0;
			tmp->height=0;
			tmp2->height=1;
		}
		else if(ft*tmp->diff()>0){
			node->bridge(tmp);

			tmp->*branch[lr]=node;
			node->*branch[clr]=tmp2;
			tmp2->top=node;
			
			node->updateHeight();
			tmp->updateHeight();
			tmp2=tmp;
		}
		else{
			node->bridge(tmp2);
			node->*branch[clr]=tmp2->*branch[lr];
			if(tmp2->*branch[lr]){
				(tmp2->*branch[lr])->top=node;
			}
			tmp2->*branch[lr]=node;

			tmp->*branch[lr]=tmp2->*branch[clr];
			if(tmp2->*branch[clr]){
				(tmp2->*branch[clr])->top=tmp;
			}
			tmp2->*branch[clr]=tmp;
			tmp->top=tmp2;

			node->updateHeight();
			tmp->updateHeight();
			tmp2->updateHeight();
		}
	}
	
	if(isRoot){
		Root=tmp2;
	}
	if(isRelRoot){
		tmp2->up=tmpUp;
		if(!isRoot){
			tmpUp->down=tmp2;
		}
	}
	
	/*
	Although it is not needed it for insertion, it is needed
	for a case when a deletion occurs
	*/
	int d=tmp->diff();
	if(std::abs(d)>1){
		balance(tmp, diff);
	}	
}

//====================================================================

template<typename T>
int Index<T>::insert(Node<T>* &root, DataContainer<T>* tmp, int deep){
	int x=1;
	if(key.compare(tmp->data, root->data_cont->data, deep)){
		if(root->left){
			x+=insert(root->left, tmp, deep);
		}
		else{
			Node<T>* node=new Node<T>(tmp, !isUnique);
			root->left=node;
			node->top=root;
		}
	}
	else if(key.compare(root->data_cont->data, tmp->data, deep)){
		if(root->right){
			x+=insert(root->right, tmp, deep);
		}
		else{
			Node<T>* node=new Node<T>(tmp, !isUnique);
			root->right=node;
			node->top=root;
		}
	}
	else{
		if(++deep<key.length()){
			if(!root->down){
				root->down=new Node<T>(root->data_cont, !isUnique);
				root->down->up=root;
			}
			insert(root->down, tmp, deep);
		}
		else if(!isUnique){
			(root->storage)->push_back(tmp->data);
		}
		else{
			if(deleteResource){
				TReF<TT> tRef(tmp->data);
				if(std::is_same<T,TT*>::value){
					delete tRef();
					delete tmp;
				}
			}
			throw "Index is set as unique, unable to insert duplicate key";
		}
		return 0;
	}
	
	root->height=std::max(root->height, x);

	int diff=root->diff();
	if(std::abs(diff)>1){
		balance(root, diff);
	}
	
	return root->height;
}

//====================================================================

template<typename T>
void Index<T>::insert(const T& val){
	DataContainer<T>* tmp=new DataContainer<T>(val);
	if(Root){
		insert(Root, tmp, 0);
	}
	else{
		Root=new Node<T>(tmp, !isUnique);
		
	}
}

//====================================================================

template<typename T>
void Index<T>::insert(T&& val){
	DataContainer<T>* tmp=new DataContainer<T>(std::move(val));
	if(Root){
		insert(Root, tmp, 0);
	}
	else{
		Root=new Node<T>(tmp, !isUnique);
		
	}
}

//====================================================================
/*
If val is a partial key it will return the first result
*/
//*
template<typename T>
template<typename TA>
Node<T>* Index<T>::tFind(const TA& val, int mx){
	int limit=key.length();
	if(mx && mx<limit){
		limit=mx;
	}
	int deep=0;
	Node<T>* tmp=Root;
	while(tmp){
		if(key.compare(val, tmp->data_cont->data, deep)) {
			tmp=tmp->left;
		}
		else if(key.compare(tmp->data_cont->data, val, deep)) {
			tmp=tmp->right;
		}
		else{
			if(tmp->down && ++deep<limit){
				tmp=tmp->down;
			}
			else{
				break;
			}
		}
	}

	return tmp;
}

//====================================================================

template<typename T>
bool Index<T>::remove(const T& value){
	Node<T>* node=tFind(value);	
	if(!node){
		return false;
	}

	int diff=node->diff();
	int lr, clr;
	if(diff>0){
		lr=LR::LEFT;
		clr=LR::RIGHT;
	}
	else if(diff<0){
		lr=LR::RIGHT;
		clr=LR::LEFT;
	}
	else{
		if(node->right){
			lr=LR::LEFT;
			clr=LR::RIGHT;
		}
		else{
			lr=LR::RIGHT;
			clr=LR::LEFT;
		}
	}
	bool isRoot=(Root==node);
	
	Node<T>* tmp=nullptr;
	Node<T>* tmp2=nullptr;

	if(!node->right && !node->left){
		if(isRoot){
			Root=nullptr;
			delete node;
			return true;
		}

		if(node->top->right==node){
			node->top->right=nullptr;
		}
		else{
			node->top->left=nullptr;
		}
		tmp=node->top;
		tmp2=tmp;
	}
	else{
		tmp=node->*branch[clr];
		while(tmp->*branch[lr]){
			tmp=tmp->*branch[lr];
		}

		if(tmp->top==node){
			node->bridge(tmp);
			tmp->*branch[lr]=node->*branch[lr];
			if(tmp->*branch[lr]){
				(tmp->*branch[lr])->top=tmp;
			}
			tmp2=tmp;
		}
		else{
			tmp2=tmp->top;
			tmp2->*branch[lr]=tmp->*branch[clr];
			if(tmp->*branch[clr]){
				(tmp->*branch[clr])->top=tmp2;
			}

			node->bridge(tmp);
			tmp->*branch[lr]=node->*branch[lr];
			if(node->*branch[lr]){
				(node->*branch[lr])->top=tmp;
			}
			
			tmp->*branch[clr]=node->*branch[clr];			
			if(node->*branch[clr]){
				(node->*branch[clr])->top=node->top;
			}
		}
	}
	if(isRoot){
		Root=tmp;
	}
	deleting(node, false);

	while(tmp2){
		tmp2->updateHeight();
		diff=tmp2->diff();
		if(std::abs(diff)>1){
			balance(tmp2, diff);
		}
		tmp2=tmp2->top;
	}

	return true;
}


//====================================================================

template<typename T>
template<class... Args>
T Index<T>::getUnique(Args... args){
	auto ta=mkTuple(args...);
	int x=std::tuple_size<tuple<Args...>>::value;

	void** vta=TupleToArray(ta);
	Node<T>* node=tFind(vta, x);
	if(!node){
		throw "Value not found.";
	}

	delete[] vta;
	return node->data_cont->data;
}

//====================================================================

template<typename T>
typename Index<T>::TT* Index<T>::first(){
	Node<T>* tmp=Root;
	while(tmp->left){
		tmp=tmp->left;
	}
	return tmp->data_cont->data;
}

//====================================================================		

template<typename T>
typename Index<T>::TT* Index<T>::last(){
	Node<T>* tmp=Root;
	while(1){
		while(tmp->right){
			tmp=tmp->right;
		}
		if(tmp->down){
			tmp=tmp->down;
		}
		else{
			break;
		}
	}

	return tmp->data_cont->data;
}

//====================================================================

template<typename T>
inline int Index<T>::getKeySize()const{
	return key.length();
}
			
//====================================================================

template<typename T>
template<typename S>
int Index<T>::inKey(S TT::* ptr){
	return key.inKey(ptr);
}

//====================================================================

}// END of namespace DM


#endif
