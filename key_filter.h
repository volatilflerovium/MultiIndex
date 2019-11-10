/*********************************************************************
* RULE class                                 								*
* RULEMANAGER class                            								*
* rangeRule class                              								*
* withRule class                               								*
*                                                                    *
* Version: 1.0                                                       *
* Date:    09-11-2019                                                *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef KEY_FILTER_H
#define KEY_FILTER_H

#include<vector>
#include "node.h"

using namespace std;
namespace DM{

//####################################################################

const int MAX_NUM_RULES=40;

enum class COMPARE : bool {LESS=true, GREATER=false};
enum class BOUND : bool {INCLUSIVE=true, EXCLUSIVE=false};

//####################################################################

template<typename T>
struct Node;

template<typename T>
class Rule
{
	typedef typename std::remove_pointer<T>::type TT;
	public:
		virtual bool testing(TT*)=0;
		virtual void apply(std::vector<TT*>&, Node<TT*>*)=0;
		virtual void setRule(Rule<T>**)=0;
		virtual void setFilter(Rule<T>**)=0;
};

//####################################################################
// No very good with names...

template<typename T>
class ABC 
{
	typedef typename std::remove_pointer<T>::type TT;

	protected: 
		Rule<T>** rules;
		Rule<T>** filter;		

	public:
		virtual ~ABC(){}

		ABC(Rule<T>** _rules, Rule<T>** _filter)
			: rules(_rules), filter(_filter){}

		virtual void getData(std::vector<TT*>& search_results, Node<TT*>* node){
			if(node==nullptr){
				return;
			}
			getData(search_results, node->left);
			if(node->top){
				if(applyFilter(node->data_cont->data)){
					search_results.push_back(node->data_cont->data);
				}
				if(node->storage){
					for(auto t : *(node->storage)){
						if(applyFilter(t)){
							search_results.push_back(t);
						}
					}
				}
			}
			if(node->down){
				getData(search_results, node->down);
			}
			getData(search_results, node->right);
		}				

		bool applyFilter(TT* x){
			if(this->filter!=nullptr){
				for(int i=0; i<MAX_NUM_RULES; i++){
					if(filter[i]){
						if(!filter[i]->testing(x)){
							return false;
						}
					}
				}
			}
			return true;
		}	
};

//####################################################################
// No very good with names...
template<typename T>
class RTM : public Rule<T>, public ABC<T>
{
	typedef typename std::remove_pointer<T>::type TT;

	public:
		RTM(Rule<T>** _rules, Rule<T>** _filter)
			: ABC<T>(_rules, _filter){}

		inline virtual void setFilter(Rule<T>** _filter){
			this->filter=_filter;
		}

		inline virtual void setRule(Rule<T>** sdf){
			this->rules=sdf;
		}

		void collectData(std::vector<TT*>& search_results, Node<TT*>* node){
			if(node==nullptr){
				return;
			}
			
			if((*(this->rules+1))==nullptr){
				if(!node->down){	
					if(this->applyFilter(node->data_cont->data)){
						search_results.push_back(node->data_cont->data);
					}
					if(node->storage ){
						for(auto t : *(node->storage)){
							if(this->applyFilter(t)){
								search_results.push_back(t);
							}
						}
					}
				}
				else{
					this->getData(search_results, node->down);
				}
			}
			else{
				(*(this->rules+1))->apply(search_results, node->down);
			}
		}				
};

//####################################################################

template<typename S, typename T>
class lessThanRule : public RTM<T>
{
	typedef typename std::remove_pointer<T>::type TT;
	
	private:
		S point;
		S TT::* field;

	public:
		lessThanRule(S TT::* _field, const S& p)
			:field(_field), point(p), RTM<T>(nullptr, nullptr){}

		inline virtual bool testing(TT* z){
			if(z->*field<=point){
				return true;
			}
			return false;
		}

		virtual void apply(std::vector<TT*>& search_results, Node<TT*>* node){
			if(!node){
				return;
			}
			if(point < node->data_cont->data->*field){
				apply(search_results, node->left);
			}
			else{
				apply(search_results, node->left);
				this->collectData(search_results, node);
				apply(search_results, node->right);
			}
		}
};

//####################################################################

template<typename S, typename T>
class greaterThanRule : public RTM<T>
{
	typedef typename std::remove_pointer<T>::type TT;
	
	private:
		S point;
		S TT::* field;

	public:
		greaterThanRule(S TT::* _field, const S& p)
			:field(_field), point(p), RTM<T>(nullptr, nullptr){}

		inline virtual bool testing(TT* z){
			if(point<=z->*field){
				return true;
			}
			return false;
		}

		virtual void apply(std::vector<TT*>& search_results, Node<TT*>* node){
			if(!node){
				return;
			}
			if(node->data_cont->data->*field<point){
				apply(search_results, node->right);
			}
			else{
				apply(search_results, node->left);
				this->collectData(search_results, node);
				apply(search_results, node->right);
			}
		}
};

//####################################################################

template<typename S, typename T>
class rangeRule : public RTM<T>
{
	typedef typename std::remove_pointer<T>::type TT;
	
	private:
		S left;
		S right;
		S TT::* field;

	public:
		rangeRule(S TT::* _field, const std::pair<S, S>&pr)
			:field(_field), left(pr.first), right(pr.second), RTM<T>(nullptr, nullptr){}

		inline virtual bool testing(TT* z){
			if(left<=z->*field && z->*field<=right){
				return true;
			}
			return false;
		}

		virtual void apply(std::vector<TT*>& search_results, Node<TT*>* node){
			if(!node){
				return;
			}
			if(node->data_cont->data->*field < left){
				apply(search_results, node->right);
			}
			else if(right < node->data_cont->data->*field){
				apply(search_results, node->left);
			}
			else{
				apply(search_results, node->left);
				this->collectData(search_results, node);
				apply(search_results, node->right);
			}
		}
};

//####################################################################

template<typename S, typename T>
class withRule : public RTM<T>
{
	typedef typename std::remove_pointer<T>::type TT;
	private:
		std::vector<S> points;
		S TT::* field;
	
	public:
		withRule(S TT::* _field, std::initializer_list<S> ls)
			:field(_field), RTM<T>(nullptr, nullptr){
			for(S t : ls){		
				points.push_back(t);
			}
		}

		withRule(S TT::* _field, std::vector<S>&& ls)
			:field(_field), points(std::move(ls)), RTM<T>(nullptr, nullptr){}			

		withRule(S TT::* _field, const std::vector<S>& ls)
			:field(_field), points(ls), RTM<T>(nullptr, nullptr){}

		virtual bool testing(TT* z){
			for(int i=0; i<points.size(); i++){
				if(z->*field==points[i]){
					return true;
				}
			}	
			return false;
		}

		virtual void apply(std::vector<TT*>& search_results, Node<TT*>* node){
			if(!node){
				return;
			}
			Node<TT*>* tmp=node;
			for(int i=0; i<points.size(); i++){
				tmp=node;
				while(tmp){
					if(points[i] < tmp->data_cont->data->*field){
						tmp=tmp->left;
					}
					else if(tmp->data_cont->data->*field < points[i]) {
						tmp=tmp->right;
					}
					else{
						break;
					}
				}
				if(tmp){
					this->collectData(search_results, tmp);
				}
			}	
		}
};

//####################################################################

template<typename T>
class RuleManager : public ABC<T>
{
	typedef typename std::remove_pointer<T>::type TT;
	typedef Rule<T> RT;
	private:		
		int count;
		int filter_count;

	public:
		~RuleManager(){
			for(int i=0; i<count; i++){
				delete this->rules[i];
			}
			for(int i=0; i<filter_count; i++){
				delete this->filter[i];
			}
			delete[] this->rules;
			delete[] this->filter;
		}

		RuleManager()
			:count(0), filter_count(0), ABC<T>(new Rule<T>*[MAX_NUM_RULES], new Rule<T>*[MAX_NUM_RULES]){
			for(int i=0; i<MAX_NUM_RULES; i++){
				this->rules[i]=nullptr;
				this->filter[i]=nullptr;
			}
		}

		int insertRule(Rule<T>* rl,bool isFilter=false){
			Rule<T>** tmp=this->rules;
			int ct, a;
			if(isFilter){
				tmp=this->filter;
				ct=filter_count++;
				a=-1;
			}
			else{
				ct=count++;
				a=ct;
			}
			if(ct<MAX_NUM_RULES){
				tmp[ct]=rl;
			}
			return a;
		}

		template<typename S>
		Rule<T>* mkRule(S TT::* field, const S& p, COMPARE lsGt){
			if(COMPARE::LESS==lsGt){
				return new lessThanRule<S, T>(field, p);
			}
			return new greaterThanRule<S, T>(field, p);
		}

		template<typename S>
		Rule<T>* mkRule(S TT::* field, std::pair<S, S>& p){
			return new rangeRule<S, T>(field, p);
		}

		template<typename S>
		Rule<T>* mkRule(S TT::* field, std::pair<S, S>&& p){
			return new rangeRule<S, T>(field, std::move(p));
		}					

		template<typename S>
		Rule<T>* mkRule(S TT::* field, std::vector<S>& p){
			return new withRule<S, T>(field, p);
		}

		template<typename S>
		Rule<T>* mkRule(S TT::* field, std::vector<S>&& p){
			return new withRule<S, T>(field, std::move(p));
		}

		template<typename S>
		Rule<T>* mkRule(S TT::* field, std::initializer_list<S> ls){	
			return new withRule<S, T>(field, std::move(ls));
		}

		void getResult(std::vector<TT*>& search_results, Node<TT*>* root){
			this->rules[0]->apply(search_results, root);
		}

		void sortKey(int* order, int mx){
			Rule<T>** tmp=new Rule<T>*[mx];
			for(int j=0; j<mx; j++){
				tmp[j]=this->rules[order[j]];
				this->rules[order[j]]=nullptr;
			}

			for(int j=0; j<MAX_NUM_RULES; j++){
				if(this->rules[j]){ 
				   /*
				   READ ME:  we do not need to set the filter in those rules that are filter, 
				   	because they do not call themselves.
				   */
					this->filter[filter_count++]=this->rules[j];
					this->rules[j]=nullptr;	
				}
			}
			for(int j=0; j<mx; j++){
				this->rules[j]=tmp[j];
				this->rules[j]->setRule(this->rules+j);
				this->rules[j]->setFilter(this->filter);
				tmp[j]=nullptr;
			}
			count=mx;
			delete[] tmp;	
		}
};

//####################################################################
//####################################################################


}

#endif