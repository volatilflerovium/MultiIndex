/*********************************************************************
* KeyComPound class                                						*
*                                                                    *
* Version: 1.0                                                       *
* Date:    09-11-2019                                                *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef KEY_H
#define KEY_H

#include <algorithm>

using namespace std;

namespace DM{

//####################################################################

template<typename S>
std::tuple<S> mkTuple(S s){
	return std::make_tuple(s);
}

template<typename S, typename... Args>
std::tuple<S, Args...> mkTuple(S s, Args... args){
	return std::tuple_cat(std::make_tuple(s), mkTuple(args...));
}

//####################################################################

template<typename Tuple, std::size_t N, std::size_t B>
struct TtoV
{
	static void convert(void** vx, Tuple& tuple){
		vx[N-1]=&std::get<N-1>(tuple);
		TtoV<Tuple, N+1, B>::test(vx, tuple);
	}
};

template<typename Tuple, std::size_t B>
struct TtoV<Tuple, B, B>
{
	static void convert(void** vx, Tuple& tuple){
		vx[B-1]=&std::get<B-1>(tuple);
	}
};

template<typename Tuple>
void** TupleToArray(Tuple& tuple)
{
	void** vx=new void*[std::tuple_size<Tuple>::value];
	TtoV<Tuple, 1, std::tuple_size<Tuple>::value>::convert(vx, tuple);
	return vx;
}

//####################################################################
//####################################################################

template<typename D, typename T, typename S>
bool pTest(T D::* t, S D::* s){
	if(std::is_same<T,S>::value){
		if(s==reinterpret_cast <S D::*>(t)){
			return true;
		}
	}
	return false;
}

//====================================================================

template<typename T>
class Keyx
{
	public:
		
		virtual bool testing(const T& x, const T& y)=0;
		virtual bool testing(T*& x, T*& y)=0;

		virtual bool testing(const T& x, void* y)=0;
		virtual bool testing(void* y, const T& x)=0;

		virtual bool testing(T*& x, void*& y)=0;
		virtual bool testing(void*& y, T*& x)=0;
		
		virtual bool sameField(void* data)=0;
};

//====================================================================
//====================================================================
//====================================================================

template<typename S, typename T>
class KeyP : public Keyx<T>
{
	//typedef typename std::remove_pointer<T>::type TT;
	typedef KeyP<S, T> KyP;
	private:
		S T::* field;

	public:
		KeyP(S T::* _field)
			:field(_field){}

		S T::* returnKey(){
			return field;
		}

		virtual bool testing(const T& x, const T& y){
			return (x.*field < y.*field);
		}

		virtual bool testing(T*& x, T*& y){
			return (x->*field < y->*field);
		}

		virtual bool testing(const T& x, void* y){
			return (x.*field < *(static_cast<S*>(y)));
		};

		virtual bool testing(void* y, const T& x){
			return (*(static_cast<S*>(y)) < x.*field);
		};

		virtual bool testing(T*& x, void*& y){
			return (x->*field < *(static_cast<S*>(y)));
		};

		virtual bool testing(void*& y, T*& x){
			return (*(static_cast<S*>(y)) < x->*field);
		};

		bool sameField(void* data){
			auto tt=reinterpret_cast<KyP*>(data);
			return pTest(field, tt->returnKey());
		}
};

//====================================================================
//====================================================================
//====================================================================

template<typename T>
class KeyCompound
{
	typedef typename std::remove_pointer<T>::type TT;
	private:
		static const int max_tests=40;
		Keyx<T>* keys[max_tests];
		int count;

	public:
		~KeyCompound(){
			for(int i=0; i<count; i++){
				delete keys[i];
			}
		}

		KeyCompound()
			:count(0){}

		int length() const{
			return count;
		}

		template<typename S>
		void insertKey(S T::* field){
			if(count<max_tests){
				keys[count++]=new KeyP<S, T>(field);
			}
		}

		bool compare(const T& x, const T& y, int deep){
			return keys[deep]->testing(x, y);
		}

		bool compare(T* x, T* y, int deep){
			return keys[deep]->testing(x, y);
		}

		bool compare(const T& x, void** vd, int deep){
			return keys[deep]->testing(x, vd[deep]);
		}

		bool compare(T* x,  void** vd, int deep){
			return keys[deep]->testing(x, vd[deep]);
		}

		bool compare(void** vd, const T& x, int deep){
			return keys[deep]->testing(vd[deep], x);
		}

		bool compare(void** vd, T* x, int deep){
			return keys[deep]->testing(vd[deep], x);
		}

		template<typename S>
		bool compare(const T& x, const S& s, int deep){
			void* y=&s;
			return keys[deep]->testing(x, y);
		}

		template<typename S>
		bool compare(const S& s, const T& x, int deep){
			void* y=&s;
			return keys[deep]->testing(y, x);
		}

		template<typename S>
		bool compare(T* x, const S& s, int deep){
			return compare(*x, s, deep);
		}

		template<typename S>
		bool compare(const S& s, T* x, int deep){
			return compare(s, *x, deep);
		}

		template<typename S>
		int inKey(S T::* ptr);
};

template<typename T>
template<typename S>
int KeyCompound<T>::inKey(S T::* ptr){
	KeyP<S, T>* tts=new KeyP<S, T>(ptr);
	void* tt=tts;
	int j=-1;
	for(int i=0; i<count; i++){
		if(keys[i]->sameField(tt)){
			j=i;
		}
	}
	delete tts;
	return j;
}

//====================================================================

template<typename D,typename T>
void mkKey(KeyCompound<D>& key, T v) {
	key.insertKey(v);
}

//====================================================================

template<typename D, typename T, typename... Args>
void mkKey(KeyCompound<D>& key, T t, Args... args) {
	key.insertKey(t);
	mkKey(key, args...);

}

//####################################################################

}

#endif
