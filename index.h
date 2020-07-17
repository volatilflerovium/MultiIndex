/*********************************************************************
* Index class                                								*
*                                                                    *
* Version: 1.1                                                       *
* Date:    16-07-202                                                 *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef INDEX_H
#define INDEX_H

#include "key.h"
#include "node.h"
#include "utility.h"
#include "container.h"

using namespace Allocator;

namespace DM{

//####################################################################
//####################################################################

template<typename T>
class Query;

template<typename T>
class Index
{
	typedef typename std::remove_pointer<T>::type TT;
	typedef Node<TT*> NodeT;

	private:
		enum LR{LEFT, RIGHT};
		Node<TT*>* Node<TT*>::* branch[2]={&Node<TT*>::left, &Node<TT*>::right};
		KeyCompound<TT> key;
		INDEX_TYPE isUnique;
		Node<TT*>* current_node;
		TT* tmp_data;
		static Container<T>* mallocator;
		Int2Type<Is_Pointer<T>::is_pointer> int2type;

		void balance(Node<TT*>* node, int diff);

		int insert(Node<TT*>* &, int);

		template<typename TA>
		Node<TT*>* tFind(const TA& value, int mx=0);

		void deleting(Node<TT*>* node);
		
		bool removeNode(Node<TT*>* node);
		
		Node<TT*>* Root;

		template<typename S>
		int inKey(S TT::* ptr);

		Index<T>()=delete;
		Index<T>& operator=(const Index<T>&)=delete;

	public:

		template<typename S, typename... Args>
		Index(INDEX_TYPE unique, S TT::* s, Args... args);

		~Index();

		static void set_allocator(Container<T>* _mallocator);

		bool load_data();

		void print(Node<TT*>* root=nullptr)const;

		void insert(TT* val_ptr);

		bool remove(TT* ptr);

		template<class... Args>
		TT* getUnique(Args... args);
		
		template<class... Args>
		Node<TT*>* find(Args... args);

		TT* first();
		TT* last();

		int getKeySize()const;
		
	friend class Query<T>;
};

//====================================================================

template<typename T>
Container<T>* Index<T>::mallocator(nullptr);

//====================================================================

template<typename T>
template<typename S, typename... Args>
Index<T>::Index(INDEX_TYPE _unique, S TT::* s, Args... args)
:Root(nullptr), isUnique(_unique), tmp_data(nullptr)
{
	mkKey(key, s, args...);
}

//====================================================================

template<typename T>
void Index<T>::set_allocator(Container<T>* _mallocator){
	mallocator=_mallocator;
}

//====================================================================

template<typename T>
Index<T>::~Index(){
	deleting(Root);
}

//====================================================================

template<typename T>
void Index<T>::deleting(Node<TT*>* node){
	if(node){
		Node<TT*>* tmp=node->right;
		Node<TT*>* tmp2=node->left;

		mallocator->free_node(node);

		deleting(tmp);
		deleting(tmp2);
	}
}

//====================================================================

template<typename T>
void Index<T>::insert(TT* val_ptr){
	tmp_data=val_ptr;
	if(Root){
		insert(Root, 0);
	}
	else{
		Root=mallocator->new_node(tmp_data, !isUnique);
	}
}

//====================================================================

template<typename T>
int Index<T>::insert(Node<TT*>* &root,  int deep){
	int x=1;
	if(key.compare(tmp_data, root->data, deep)){
		if(root->left){
			x+=insert(root->left, deep);
		}
		else{
			root->left=mallocator->new_node(tmp_data, !isUnique);		
			root->left->top=root;
		}
	}
	else if(key.compare(root->data, tmp_data, deep)){
		if(root->right){
			x+=insert(root->right, deep);
		}
		else{
			root->right=mallocator->new_node(tmp_data, !isUnique);		
			root->right->top=root;			
		}
	}
	else{
		if(++deep<key.length()){			
			if(!root->down){
				root->forkNode(mallocator->new_node(root->data, !isUnique));		
			}
			insert(root->down, deep);
		}
		else if(!isUnique){
			(root->storage)->push_back(tmp_data);
		}
		else{
			throw "Index is set as unique, duplicate key unable to insert element.";
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
void Index<T>::balance(Node<TT*>* node, int diff){	
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
	
	Node<TT*>* tmp=node->*branch[clr];
	Node<TT*>* tmp2=tmp->*branch[lr];

	bool isRoot=(Root==node);
	bool isRelRoot=(node->top==nullptr);// && node!=Root);

	Node<TT*>* tmpUp=nullptr;
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
		if(tmpUp){
			tmpUp->data=tmp2->data;
		}
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
/*
If val is a partial key it will return the first result
*/
//*
template<typename T>
template<typename TA>
typename Index<T>::NodeT* Index<T>::tFind(const TA& val, int mx){
	int limit=key.length();
	if(mx && mx<limit){
		limit=mx;
	}
	int deep=0;
	Node<TT*>* tmp=Root;
	while(tmp){
		if(key.compare(val, tmp->data, deep)) {
			tmp=tmp->left;
		}
		else if(key.compare(tmp->data, val, deep)) {
			tmp=tmp->right;
		}
		else{
			if(++deep<limit){
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
bool Index<T>::removeNode(Node<TT*>* node){
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
	bool isRelroot=(node->top==nullptr);

	Node<TT*>* tmp=nullptr;
	Node<TT*>* tmp2=nullptr;

	if(!node->left && !node->right){
		if(isRoot){
			Root=nullptr;
			mallocator->free_node(node);
			return true;
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

	mallocator->free_node(node);
	mallocator->deallocate_node(node);

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
bool Index<T>::remove(TT* ptr){
	Node<TT*>* node=tFind(ptr);

	Node<TT*>* ankor=nullptr;
	if(node->left){
		ankor=node->left;
	}
	else{
		ankor=node->right;
	}

	Node<TT*>* tmpUp=nullptr;
	if(node->up){
		tmpUp=node->up;
		node->up=nullptr;
		tmpUp->down=nullptr;
	}

	Node<TT*>* tmpDown=nullptr;
	if(node->down){
		tmpDown=node->down;
		node->down=nullptr;
		tmpDown->up=nullptr;
	}
	
	bool a=removeNode(node);
	if(a){
		if(ankor){
			while(ankor->top){
				ankor=ankor->top;
			}
			if(tmpUp){
				ankor->up=tmpUp;
				tmpUp->down=ankor;
				while(tmpUp){
					tmpUp->data=ankor->data;
					tmpUp=tmpUp->up;
				}
			}
			if(tmpDown){
				ankor->down=tmpDown;
				tmpDown->up=ankor;
			}
		}
		else{
			if(tmpUp && tmpDown){
				tmpUp->down=tmpDown;
				tmpDown->up=tmpUp;
			}
		}
	}
	
	return a;
}


//====================================================================

template<typename T>
template<class... Args>
typename Index<T>::TT* Index<T>::getUnique(Args... args){
	Node<TT*>* node=find(std::forward<Args>(args)...);
	if(!node){
		throw "Value not found.";
	}
	return node->data;
}

//====================================================================

template<typename T>
template<class... Args>// values for the elements of the key
Node<typename Index<T>::TT*>* Index<T>::find(Args... args){
	std::tuple<Args...> ta=mkTuple(std::forward<Args>(args)...);

	void** vta=TupleToArray(ta);
	Node<TT*>* node=tFind(vta, std::tuple_size<tuple<Args...>>::value);
	delete[] vta;
	return node;
}

//====================================================================

template<typename T>
typename Index<T>::TT* Index<T>::first(){
	Node<TT*>* tmp=Root;
	while(tmp->left){
		tmp=tmp->left;
	}
	return tmp->data;
}

//====================================================================		

template<typename T>
typename Index<T>::TT* Index<T>::last(){
	Node<TT*>* tmp=Root;
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

	return tmp->data;
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

template<typename T>
void Index<T>::print(Node<TT*>* root)const{
	std::list<Node<TT*>*> NL; 
	std::list<Node<TT*>*> SBT; 
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
				std::cout<<*((*it)->data)<<" : ";
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
	std::cout<<"\n\n+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +\n\n";
	
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
bool Index<T>::load_data(){
	try{
		mallocator->apply_on([&](unsigned char* p_data)->void{
			insert(mallocator->get_data(p_data, int2type));
		});
	}
	catch(const char* msg){
		std::cout<<msg<<std::endl;
		return false;
	}
	return true;
}

//====================================================================


}// END of namespace DM


#endif
