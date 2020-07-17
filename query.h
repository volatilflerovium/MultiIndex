/*********************************************************************
* DM::Query class                               							*
* DM::LazyTree class                                                 *
* DM::ContainerWrapper interface                                     *
* DM::Rule interface                                                 *
* DM::Query::ABC class                                               *
* DM::Query::RTM class                                               *
* DM::Query::extFilterRule class                                     *
* DM::Query::trivialRule class                                       *
* DM::Query::lessThanRule class                                      *
* DM::Query::greaterThenRule class                                   *
* DM::Query::rangeRule class                                         *
* DM::Query::withRule class                                          *
* DM::Query::withRuleNot class                                       *
* DM::Query::RuleManager class                                       *
*                                                                    *
* Version: 1.1                                                       *
* Date:    16-07-202                                                 *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/

#ifndef QUERY_H
#define QUERY_H
#include <functional>
#include <vector>

#include "index.h"

using namespace std;

namespace DM{

//####################################################################

const int MAX_NUM_RULES=40;

enum class COMPARE : bool {LESS=true, GREATER=false};

enum class BOUND : bool {INCLUSIVE=true, EXCLUSIVE=false};

//####################################################################

template<typename S>
struct LazyTree
{
	std::vector<S> vect;

	LazyTree(std::vector<S>&& _vect)
	:vect(std::move(_vect)){
		std::sort(vect.begin(), vect.end()); 
	}

	LazyTree(const std::vector<S>& _vect)
	:vect(_vect){
		std::sort(vect.begin(), vect.end()); 
	}

	bool find(const S& s);
};

template<typename S>
bool LazyTree<S>::find(const S& s){
	int n=vect.size()/2;
	int t=n, p=-1, q=vect.size();
	while(q-p>1){			
		if(s<vect[t]){
			q=t;
			n=-1;
		}
		else if(vect[t]<s){
			p=t;
			n=1;
		}
		else{
			return true;
		}			
		t+=n*std::max((q-p-1)/2, 1);
	}
	return false;
}

//####################################################################

template<typename T>
class ContainerWrapper
{
	typedef typename std::remove_pointer<T>::type TT;
	typedef std::function<bool(TT*)> ExtFilter;

	private:
		ExtFilter exFilter;

	public:
		ContainerWrapper(ExtFilter filter)
		: exFilter(std::move(filter))
		{}

		virtual void insert(TT*)=0;

		bool filter(TT* data){
			if(exFilter(data)){
				insert(data);
				return true;
			}
			return false;
		}
};

template<typename T>
class VContainerWrapper : public ContainerWrapper<T>
{
	typedef typename std::remove_pointer<T>::type TT;
	typedef std::function<bool(TT*)> ExtFilter;
	
	private:
		std::vector<TT*>* container;		

	public:	
		VContainerWrapper(std::vector<TT*>* cnt, ExtFilter filter=[](TT*)->bool{return true;})
		:ContainerWrapper<T>(filter),
		container(cnt)
		{}
		
		virtual void insert(TT* t){
			container->push_back(t);
		}
};

template<typename T>
class IContainerWrapper : public ContainerWrapper<T>
{
	typedef typename std::remove_pointer<T>::type TT;
	typedef std::function<bool(TT*)> ExtFilter;

	private:
		Index<T>* container;

	public:
		IContainerWrapper(Index<T>* cnt, ExtFilter filter=[](TT*)->bool{return true;})
		:ContainerWrapper<T>(filter), 
		 container(cnt) 
		{}

		virtual void insert(TT* t){
			container->insert(t);
		}
};

//####################################################################

template<typename T>
class Rule
{
	typedef typename std::remove_pointer<T>::type TT;
	public:
		virtual bool testing(TT*)=0;
		virtual void apply(ContainerWrapper<T>* , Node<TT*>*, bool recursive=true)=0;
		virtual void setRule(Rule<T>**)=0;
		virtual void setFilter(Rule<T>**)=0;
};

//####################################################################
//####################################################################
//####################################################################

template<typename T>
class Query
{
	typedef typename std::remove_pointer<T>::type TT;
	typedef std::function<bool(TT*)> ExtFilter;

	private:

		// No very good with names...
		template<typename P>
		class ABC 
		{
			typedef typename std::remove_pointer<P>::type PP;
		
			protected: 
				Rule<P>** rules;
				Rule<P>** filter;		
				void extractData(ContainerWrapper<P>* search_results, Node<PP*>* node);
		
			public:
				virtual ~ABC(){}
		
				ABC(Rule<P>** _rules, Rule<P>** _filter)
					: rules(_rules), filter(_filter){}
		
				void getData(ContainerWrapper<P>* search_results, Node<PP*>* node);
				bool applyFilter(PP* x);
		};
		
		// No very good with names...
		template<typename P>
		class RTM : public Rule<P>, public ABC<P>
		{
			typedef typename std::remove_pointer<P>::type PP;
		
			public:
				RTM(Rule<P>** _rules, Rule<P>** _filter)
					: ABC<P>(_rules, _filter){}
		
				virtual void setFilter(Rule<P>** _filter){
					this->filter=_filter;
				}
		
				virtual void setRule(Rule<P>** sdf){
					this->rules=sdf;
				}
		
				void collectData(ContainerWrapper<P>* search_results, Node<PP*>* node);
		};

		template<typename R, typename P>
		class extFilterRule : public RTM<P>
		{
			typedef typename std::remove_pointer<P>::type PP;
			typedef std::function<bool(PP*)> ExtFilter;
		
			private:
				ExtFilter extFilter;
				R PP::* field;
		
			public:
				extFilterRule(R PP::* _field, ExtFilter filter)
					:RTM<P>(nullptr, nullptr), field(_field), extFilter(filter) {}
		
				virtual bool testing(PP* z){
					return extFilter(z);
				}
		
				virtual void apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive=true);
		};

		template<typename P>
		class trivialRule : public RTM<P>
		{
			typedef typename std::remove_pointer<P>::type PP;
		
			public:
				trivialRule()
					:RTM<P>(nullptr, nullptr){}
		
				virtual bool testing(PP* z){
					return true;
				}
		
				virtual void apply(ContainerWrapper<P>* search_results, Node<PP*>* node, bool recursive=true);
		};

		template<typename R, typename P>
		class lessThanRule : public RTM<P>
		{
			typedef typename std::remove_pointer<P>::type PP;
			
			private:
				R point;
				R PP::* field;
		
			public:
				lessThanRule(R PP::* _field, R&& p)
					:RTM<P>(nullptr, nullptr), field(_field), point(std::forward<R>(p)){}
		
				virtual bool testing(PP* z){
					if(z->*field<=point){
						return true;
					}
					return false;
				}
		
				virtual void apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive=true);
		};

		template<typename R, typename P>
		class greaterThanRule : public RTM<P>
		{
			typedef typename std::remove_pointer<P>::type PP;
			
			private:
				R point;
				R PP::* field;
		
			public:
				greaterThanRule(R PP::* _field, R&& p)
					:RTM<P>(nullptr, nullptr), field(_field), point(std::forward<R>(p)){}
		
				inline virtual bool testing(PP* z){
					if(point<=z->*field){
						return true;
					}
					return false;
				}
		
				virtual void apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive=true);
		};

		template<typename R, typename P>
		class rangeRule : public RTM<P>
		{
			typedef typename std::remove_pointer<P>::type PP;
			
			private:
				R left;
				R right;
				R PP::* field;
		
			public:
				rangeRule(R PP::* _field, R&& pl, R&& pr)
					:RTM<P>(nullptr, nullptr), 
					left(std::forward<R>(pl)), right(std::forward<R>(pr)), field(_field)
					{}
		
				inline virtual bool testing(PP* z){
					if(left<=z->*field && z->*field<=right){
						return true;
					}
					return false;
				}
		
				virtual void apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive=true);
		};

		template<typename R, typename P>
		class withRule : public RTM<P>
		{
			typedef typename std::remove_pointer<P>::type PP;
		
			private:
				LazyTree<R> points;
				R PP::* field;
			
			public:
		
				withRule(R PP::* _field, const std::vector<R>& ls)
					:RTM<P>(nullptr, nullptr), points(ls), field(_field) {}
		
				withRule(R PP::* _field, std::vector<R>&& ls)
					:RTM<P>(nullptr, nullptr), points(std::move(ls)), field(_field) {}			
		
				virtual bool testing(PP* z){
					return points.find(z->*field);
				}
		
				virtual void apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive=true);
		};

		template<typename R, typename P>
		class withRuleNot : public RTM<P>
		{
			typedef typename std::remove_pointer<P>::type PP;
			private:
				LazyTree<R> points;
				R PP::* field;

			public:
				withRuleNot(R TT::* _field, const std::vector<R>& ls)
					:RTM<P>(nullptr, nullptr), points(ls), field(_field) {}
		
				withRuleNot(R TT::* _field, std::vector<R>&& ls)
					:RTM<P>(nullptr, nullptr), points(std::move(ls)), field(_field) {}			
		
				virtual bool testing(PP* z){
					return !points.find(z->*field);
				}
		
				virtual void apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive=true);
		};

		template<typename P>
		class RuleManager : public ABC<P>
		{
			typedef typename std::remove_pointer<P>::type PP;
			typedef std::function<bool(PP*)> ExtFilter;
		
			typedef Rule<P> RT;
		
			private:		
				int count;
				int filter_count;
		
			public:
				~RuleManager();
		
				RuleManager();
		
				int insertRule(Rule<P>* rl, bool isFilter=false);
		
				template<typename R>
				Rule<P>* mkRule(R PP::* field, R&& p, COMPARE lsGt){
					if(COMPARE::LESS==lsGt){
						return new lessThanRule<R, P>(field, std::forward<R>(p));
					}
					return new greaterThanRule<R, P>(field, std::forward<R>(p));
				}
		
				template<typename R>
				Rule<P>* mkRule(R PP::* field, R&& pl, R&& pr){
					return new rangeRule<R, P>(field, std::forward<R>(pl), std::forward<R>(pr));
				}					
		
				template<typename R>
				Rule<P>* mkRule(R PP::* field, const std::vector<R>& ls, bool equal=true){	
					if(equal){
						return new withRule<R, P>(field, ls);
					}
					return new withRuleNot<R, P>(field, ls);
				}
		
				template<typename R>
				Rule<P>* mkRule(R PP::* field, std::vector<R>&& ls, bool equal=true){	
					if(equal){
						return new withRule<R, P>(field, std::move(ls));
					}
					return new withRuleNot<R, P>(field, std::move(ls));
				}
		
				template<typename R>
				Rule<P>* mkRule(R PP::* field, ExtFilter filter){	
					return new extFilterRule<R, P>(field, filter);
				}
		
				Rule<P>* mkRule(){	
					return new trivialRule<P>();
				}
		
				void getResult(ContainerWrapper<P>* search_results, Node<PP*>* root){
					this->rules[0]->apply(search_results, root);
				}
		
				void sortKey(int* order, int mx);
		};

	private:
		RuleManager<TT>* queryKey;
		Index<T>* mIndex;
		DBTable<T>* MIndex;
		std::vector<int*> rank;
		bool ready;
		void clear();
		void setup_query();
		bool ruleLoaded;
		template<typename S>
		void insertRule(S TT::* field, Rule<TT>* rule);
		
		bool getBestIndex();
		void executeQuery(ContainerWrapper<T>* results);
		
		Query()=delete;
		Query<T>& operator=(const Query<T>&)=delete;

	public:

		Query(DBTable<T>* multiIndex);
		~Query();

		/*
		 * select elements that are less/greater (lsGt) than p
		 * */
		template<typename S>
		Query<T>& selectWhere(S TT::* field, S&& p, COMPARE lsGt);

		/*
		 * select elements that are in the range pl - pr 
		 * (between pl and pr inclusive)
		 * */
		template<typename S>
		Query<T>& selectWhere(S TT::* field, S&& pl, S&& pr);
		
		/*
		 * select elements whose member field is equal to one in the array
		 * ({x in collection where x->*field in vp})
		 * */
		template<typename S>
		Query<T>& selectWhere(S TT::* field, const std::vector<S>& vp);

		/*
		 * same as above
		 * */
		template<typename S>
		Query<T>& selectWhere(S TT::* field, std::vector<S>&& lvp);

		/*
		 * select element that are not in the list
		 * */
		template<typename S>
		Query<T>& selectWhereNot(S TT::* field, const std::vector<S>& vp);

		/*
		 * same as above
		 * */
		template<typename S>
		Query<T>& selectWhereNot(S TT::* field, std::vector<S>&& vp);

		template<typename S>
		Query<T>& selectUsingFilter(S TT::* field,  ExtFilter filter);

		Query<T>& selectAll();
		
		bool statementSet(){
			return ruleLoaded;
		}

		std::vector<TT*> getResults();

		template<typename S>
		Index<T>* getResults(S TT::* field);
		
		template<typename R, typename S>
		std::vector<std::tuple<TT*, R*>> innerJoin(Query<R>* exQuery, S R::* r, S T::* v);

		template<typename S, typename R, typename... Args>
		std::vector<std::tuple<TT*, R*, Args...>> innerJoin(const std::vector<std::tuple<R*, Args...>>& vect, S R::* r, S T::* t);

		template<class... Args>
		TT* getUnique(const std::string& indexName, Args&&... args);
		
};

//====================================================================
//====================================================================
//====================================================================

template<typename T>
template<typename P>
void Query<T>::ABC<P>::extractData(ContainerWrapper<P>* search_results, Node<PP*>* node){
	if(applyFilter(node->data)){
		search_results->filter(node->data);
	}
	if(node->storage){
		for(auto t : *(node->storage)){
			if(applyFilter(t)){
				search_results->filter(t);
			}
		}
	}
}

template<typename T>
template<typename P>
void Query<T>::ABC<P>::getData(ContainerWrapper<P>* search_results, Node<PP*>* node){
	if(node==nullptr){
		return;
	}

	getData(search_results, node->left);
	if(node->down){
		getData(search_results, node->down);
	}
	else{
		extractData(search_results, node);
	}
	getData(search_results, node->right);
}	

//====================================================================

template<typename T>
template<typename P>
bool Query<T>::ABC<P>::applyFilter(PP* x){
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

//====================================================================

template<typename T>
template<typename P>
void Query<T>::RTM<P>::collectData(ContainerWrapper<P>* search_results, Node<PP*>* node){
	if(node==nullptr){
		return;
	}
			
	if((*(this->rules+1))==nullptr){
		if(node->down){
			this->getData(search_results, node->down);
		}
		else{
			this->extractData(search_results, node);
		}
	}
	else{
		
		if(node->down){
			(*(this->rules+1))->apply(search_results, node->down);
		}
		else{
			(*(this->rules+1))->apply(search_results, node, false);
		}
	}
}

//====================================================================

template<typename T>
template<typename R, typename P>
void Query<T>::extFilterRule<R, P>::apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive){
	if(!node){
		return;
	}
	
	if(!recursive){
		if(extFilter(node->data)){
			this->collectData(search_results, node);
		}
		return;
	}
	
	apply(search_results, node->left);
	if(extFilter(node->data)){
		this->collectData(search_results, node);
	}
	apply(search_results, node->right);
}

//====================================================================

template<typename T>
template<typename P>
void Query<T>::trivialRule<P>::apply(ContainerWrapper<P>* search_results, Node<PP*>* node, bool recursive){
	if(!node){
		return;
	}
	
	if(!recursive){
		this->collectData(search_results, node);
		return;
	}

	apply(search_results, node->left);
	this->collectData(search_results, node);
	apply(search_results, node->right);
}

//====================================================================

template<typename T>
template<typename R, typename P>
void Query<T>::lessThanRule<R, P>::apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive){
	if(!node){
		return;
	}
	
	if(!recursive){
		if(!(point < node->data->*field)){
			this->collectData(search_results, node);
		}
		return;
	}
	
	if(point < node->data->*field){
		apply(search_results, node->left);
	}
	else{
		apply(search_results, node->left);
		this->collectData(search_results, node);
		apply(search_results, node->right);
	}
}

//====================================================================

template<typename T>
template<typename R, typename P>
void Query<T>::greaterThanRule<R, P>::apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive){
	if(!node){
		return;
	}

	if(!recursive){
		if(!(node->data->*field<point)){
			this->collectData(search_results, node);
		}
		return;
	}
	
	if(node->data->*field<point){
		apply(search_results, node->right);
	}
	else{
		apply(search_results, node->left);
		this->collectData(search_results, node);
		apply(search_results, node->right);
	}
}

//====================================================================

template<typename T>
template<typename R, typename P>
void Query<T>::rangeRule<R, P>::apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive){
	if(!node){
		return;
	}

	if(!recursive){
		if(!(node->data->*field < left) && !(right < node->data->*field)){
			this->collectData(search_results, node);
		}
		return;
	}

	if(node->data->*field < left){
		apply(search_results, node->right);
	}
	else if(right < node->data->*field){
		apply(search_results, node->left);
	}
	else{
		apply(search_results, node->left);

		this->collectData(search_results, node);

		apply(search_results, node->right);
	}
}

//====================================================================

template<typename T>
template<typename R, typename P>
void Query<T>::withRule<R, P>::apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive){
	if(!node){
		return;
	}

	if(!recursive){
		if(points.find(node->data->*field)){
			this->collectData(search_results, node);
		}
		return;
	}

	Node<PP*>* tmp=node;
	bool a;
	for(int i=0; i<points.vect.size(); i++){
		tmp=node;
		a=false;
		while(tmp){
			if(points.vect[i] < tmp->data->*field){
				tmp=tmp->left;
			}
			else if(tmp->data->*field < points.vect[i]) {
				tmp=tmp->right;
			}
			else{
				a=true;
				break;
			}
		}
		if(tmp && a){
			this->collectData(search_results, tmp);
		}
	}	
}

//====================================================================

template<typename T>
template<typename R, typename P>
void Query<T>::withRuleNot<R, P>::apply(ContainerWrapper<P>*  search_results, Node<PP*>* node, bool recursive){
	if(!node){
		return;
	}
	if(!recursive){
		if(!points.find(node->data->*field)){
			this->collectData(search_results, node);
		}
		return;
	}

	apply(search_results, node->left);
	if(!points.find(node->data->*field)){
		this->collectData(search_results, node);
	}
	apply(search_results, node->right);
}

//====================================================================

template<typename T>
template<typename P>
Query<T>::RuleManager<P>::~RuleManager(){
	for(int i=0; i<count; i++){
		delete this->rules[i];
	}
	for(int i=0; i<filter_count; i++){
		delete this->filter[i];
	}
	delete[] this->rules;
	delete[] this->filter;
}

//====================================================================

template<typename T>
template<typename P>
Query<T>::RuleManager<P>::RuleManager()
	:ABC<P>(new Rule<P>*[MAX_NUM_RULES], new Rule<P>*[MAX_NUM_RULES]),
	count(0), filter_count(0){
	for(int i=0; i<MAX_NUM_RULES; i++){
		this->rules[i]=nullptr;
		this->filter[i]=nullptr;
	}
}

//====================================================================

template<typename T>
template<typename P>
int Query<T>::RuleManager<P>::insertRule(Rule<P>* rl, bool isFilter){
	Rule<P>** tmp=this->rules;
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

//====================================================================

template<typename T>
template<typename P>
void Query<T>::RuleManager<P>::sortKey(int* order, int mx){
	Rule<P>** tmp=new Rule<P>*[mx];
	for(int j=0; j<mx; j++){
		if(order[j]>-1){
			tmp[j]=this->rules[order[j]];
			this->rules[order[j]]=nullptr;
		}
		else{
			tmp[j]=new trivialRule<P>();
		}
	}

	for(int j=0; j<MAX_NUM_RULES; j++){
		if(this->rules[j]){ 
		   /*
		    * READ ME:  we do not need to set the filter in those rules that 
		    * are filter, because they do not call themselves.
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

//####################################################################
//####################################################################
//####################################################################

template<typename T>
Query<T>::~Query(){
	if(queryKey){
		delete queryKey;
	}

	for(int i=0; i<MIndex->indexesNames.size(); i++){
		delete[] rank[i];
	}
}

//====================================================================

template<typename T>
Query<T>::Query(DBTable<T>* multiIndex)
	: MIndex(multiIndex), ready(false), 
	 queryKey(nullptr), ruleLoaded(false)
{
	for(int i=0; i<MIndex->indexesNames.size(); i++){
		rank.push_back(new int[MAX_NUM_RULES]);
	}
	
}

//====================================================================

template<typename T>
void Query<T>::setup_query(){
	if(!ready){
		ready=true;
		queryKey=new RuleManager<TT>;

		for(int i=0; i<MIndex->indexesNames.size(); i++){
			for(int j=0; j<MAX_NUM_RULES; j++){
				rank[i][j]=-1;
			}
		}
	}
}

//====================================================================

template<typename T>
void Query<T>::clear(){
	if(queryKey){
		delete queryKey;
		queryKey=nullptr;
	}
	ready=false;
	ruleLoaded=false;
}

//====================================================================

template<typename T>
template<typename S>
void Query<T>::insertRule(S TT::* field, Rule<TT>* rule){
	ruleLoaded=true;
	int k=0, j=-1;
	bool a=true;

	for(int i=0; i<MIndex->indexesNames.size(); i++){
		k=(MIndex->Indexes)[i]->inKey(field);
		if(k>-1){
			if(a){
				j=queryKey->insertRule(rule);
				a=false;
			}
			rank[i][k]=j;
		}
	}

	if(a){
		queryKey->insertRule(rule, true);
	}
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectWhere(S TT::* field, S&& p, COMPARE lsGt){
	setup_query();
	insertRule(field, queryKey->mkRule(field, std::forward<S>(p), lsGt));

	return *this;
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectWhere(S TT::* field, S&& pl, S&& pr){
	setup_query();
	insertRule(field, queryKey->mkRule(field, std::forward<S>(pl), std::forward<S>(pr)));

	return *this;
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectWhere(S TT::* field, const std::vector<S>& vp){
	setup_query();
	insertRule(field, queryKey->mkRule(field, vp));

	return *this;
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectWhere(S TT::* field, std::vector<S>&& vp){
	setup_query();
	insertRule(field, queryKey->mkRule(field, std::move(vp)));

	return *this;
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectWhereNot(S TT::* field, const std::vector<S>& vp){
	setup_query();
	insertRule(field, queryKey->mkRule(field, vp, false));

	return *this;
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectWhereNot(S TT::* field, std::vector<S>&& vp){
	setup_query();
	insertRule(field, queryKey->mkRule(field, std::move(vp), false));

	return *this;
}

//====================================================================

template<typename T>
template<typename S>
Query<T>& Query<T>::selectUsingFilter(S TT::* field, ExtFilter filter){
	setup_query();
	insertRule(field, queryKey->mkRule(field, filter));

	return *this;
}

//====================================================================

template<typename T>
bool Query<T>::getBestIndex(){
	int k=0, mx=0, b=0;
	int c=0, s=0, p=MAX_NUM_RULES;
	for(int i=0; i<MIndex->indexesNames.size(); i++){
		mx=0;
		s=0;
		c=0;
		for(int j=0; j<MAX_NUM_RULES; j++){
			if(rank[i][j]>-1){
				mx++;
				s+=j-c;
				c=j;
			}
		}
		if(b<mx){
			b=mx;
			k=i;
		}
		else if(b==mx && p>s){
			p=s;
			k=i;
		}
	}
	bool usingKey=false;
	if(b>0){
		usingKey=true;
		mx=0;
		for(int j=0; j<MAX_NUM_RULES; j++){
			if(rank[k][j]>-1){
				mx=j;
			}
		}
		mx++;
	}

	//std::cout<<"using: "<<MIndex->indexesNames[k]<<"\n";

	queryKey->sortKey(rank[k], mx);

	mIndex=MIndex->Indexes[k];	

	return usingKey;
}

//====================================================================

template<typename T>
void Query<T>::executeQuery(ContainerWrapper<T>* results){
	if(getBestIndex()){
		queryKey->getResult(results, mIndex->Root);
	}
	else{
		queryKey->getData(results, mIndex->Root);
	}
	clear();	
}

//====================================================================

template<typename T>
std::vector<typename Query<T>::TT*> Query<T>::getResults(){
	std::vector<TT*> search_results;

	if(!ready){
		return search_results;
	}

	ContainerWrapper<T>* results=new VContainerWrapper<T>(&search_results);
	executeQuery(results);
	return search_results;
}

//====================================================================

template<typename T>
template<typename S>
Index<T>* Query<T>::getResults(S TT::* field){

	if(!ready){
		return nullptr;
	}

	Index<T>* search_results=new Index<T>(INDEX_TYPE::NO_UNIQUE, field);

	ContainerWrapper<T>* results=new IContainerWrapper<T>(search_results);
	executeQuery(results);
	return search_results;
}

//====================================================================

template<typename T>
template<typename R, typename S>
std::vector<std::tuple<typename Query<T>::TT*, R*>> Query<T>::innerJoin(
			Query<R>* exQuery, 
			S R::* r, 
			S T::* v
	){
	std::vector<std::tuple<TT*, R*>> exResults;
	setup_query();

	Index<T>* indexed_results=nullptr;
	bool a=true;
	if(ruleLoaded){
		indexed_results=getResults(v);
	}
	else{
		for(int i=0; i<MIndex->indexesNames.size(); i++){
			if((MIndex->Indexes)[i]->inKey(v)==0){
				a=false;
				indexed_results=(MIndex->Indexes)[i];
				break;
			}
		}
	}

	if(indexed_results){
		if(!exQuery->statementSet()){
			exQuery->selectUsingFilter(r, [indexed_results, r](R* data){
				if(indexed_results->find(data->*r)){
					return true;
				}
				return false;
			});
		}

		Node<TT*>* tmp;		
		for(auto rtmp : exQuery->getResults()){
			tmp=indexed_results->find(rtmp->*r);
			if(tmp){
				exResults.push_back(std::tuple<TT*, R*>(tmp->data, rtmp));
				if(tmp->storage){
					typename std::list<TT*>::iterator it=tmp->storage->begin();
					while(it!=tmp->storage->end()){
						exResults.push_back(std::tuple<TT*, R*>(*it, rtmp));
						it++;
					}
				}
			}
		};

		if(a){
			delete indexed_results;
		}
	}
	else{
		Index<R>* resultsR=exQuery->getResults(r);
		std::vector<TT*> search_results2;
		ContainerWrapper<T>* vectorResults=new VContainerWrapper<T>(&search_results2, [&exResults, resultsR, v](TT* t){
			auto w=resultsR->find(t->*v);
			if(w){
				exResults.push_back(std::tuple<TT*, R*>(t, w->data));
				if(w->storage){
					typename std::list<R*>::iterator it=w->storage->begin();
					while(it!=w->storage->end()){
						exResults.push_back(std::tuple<TT*, R*>(t, *it));
						it++;
					}
				}
				return true;
			}
			return false;
		});

		queryKey->getData(vectorResults, (MIndex->Indexes)[0]->Root);
		delete resultsR;
	}

	clear();
	return exResults;
}

//====================================================================

template<typename T>
template<typename S, typename R, typename... Args>
std::vector<std::tuple<typename Query<T>::TT*, R*, Args...>> Query<T>::innerJoin(
	const std::vector<std::tuple<R*, Args...>>& vect, 
	S R::* r,
	S T::* t
	){
	std::vector<std::tuple<TT*, R*, Args...>> exResults;
	setup_query();

	Index<T>* indexed_results=nullptr;
	bool a=true;
	if(!ruleLoaded){
		for(int i=0; i<MIndex->indexesNames.size(); i++){
			if((MIndex->Indexes)[i]->inKey(t)==0){
				a=false;
				indexed_results=(MIndex->Indexes)[i];
				break;
			}
		}
	}
	if(!indexed_results){
		indexed_results=new Index<T>(INDEX_TYPE::NO_UNIQUE, t);
		ContainerWrapper<T>* results=new IContainerWrapper<T>(indexed_results);
		if(ruleLoaded){
			executeQuery(results);
		}
		else{
			queryKey->getData(results, (MIndex->Indexes)[0]->Root);
		}
	}

	Node<TT*>* tmp;
	for(auto rtmp : vect){
		tmp=indexed_results->find(std::get<0>(rtmp)->*r);
		if(tmp){
			exResults.push_back(
				std::tuple_cat(std::make_tuple(tmp->data), rtmp)
			);
		}
	};

	if(a){
		delete indexed_results;
	}

	clear();
	return exResults;	
}


//====================================================================

template<typename T>
template<class... Args>
typename Query<T>::TT* Query<T>::getUnique(const std::string& indexName, Args&&... args){
	int n=MIndex->indexExist(indexName);
	if(n<0){
		throw "Index name does not exist.";
	}
	return (MIndex->Indexes[n])->getUnique(std::forward<Args>(args)...);
}


//####################################################################

}


#endif

