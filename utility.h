/*********************************************************************
* DM::Is_Pointer structure                                           *
* DM::Int2Type   structure                                           *
* DM::INDEX_TYPE enum class                                          *
*                                                                    *
* Version: 1.1                                                       *
* Date:    16-07-202                                                 *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#ifndef UTILITY_H
#define UTILITY_H

#include <fstream>
#include <cstdlib>

namespace DM{

//######################################################################
//######################################################################

template<typename T>
struct Is_Pointer
{
	static const bool is_pointer=false;
};

template<typename T>
struct Is_Pointer<T*>
{
	static const bool is_pointer=true;
};

//######################################################################

template<int N>
struct Int2Type
{
	enum{value=N};
};

//######################################################################

enum class INDEX_TYPE : bool{UNIQUE=true, NO_UNIQUE=false};

inline bool operator!(INDEX_TYPE indx){
	if(indx==INDEX_TYPE::UNIQUE){
		return false;
	}
	return true;
}


//######################################################################

std::vector<std::string> explode(const std::string& delimiter, const std::string& str){
	std::vector<std::string> words;
	int t=0;
	int p=0;
	int ds=delimiter.size();
	while(p!=std::string::npos){
		p=str.find(delimiter, t);
		words.push_back(str.substr(t, p-t));
		t=p+ds;
	}
	return words;
}

//####################################################################

template<typename Func>
void fromFile(std::string file_name, Func cbk){
	ifstream file;
	file.open(file_name);
	if(!file.is_open()){
		std::cout<<"File failed to open"<<endl;
	}
	std::string line;
	while(getline(file, line)){
		cbk(explode("#", line));
	}
}

//######################################################################

}// END of namespace DM

#endif
