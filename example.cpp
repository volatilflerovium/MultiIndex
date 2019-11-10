/*********************************************************************
* DM-DBTable class                               								*
*                                                                    *
* Version: 1.0                                                       *
* Date:    23-09-2019                                                *
* Author:  Dan Machado                                               *                                         *
**********************************************************************/
#include<iostream>
#include<math.h>
#include<list>
#include<random>
#include<algorithm>
#include<chrono>
#include<fstream>
#include<cstdlib>
#include<functional>
#include<vector>
#include<map>

#include "DBtable.h"
#include "query.h"

using namespace std;
using namespace DM;

//####################################################################
//####################################################################
//####################################################################
//####################################################################

struct Film 
{
	int id;	
	int director;
	std::string name; 
	int year; 
	int country; 
	int length; 
	int colour;
	Film(){};
	Film(int _id, int _director, std::string _name, int _year, int _country, int _length, int _colour)
	 :id(_id), director(_director), name(_name), 
	 year(_year), country(_country), length(_length), colour(_colour){};
};	 

ostream& operator<<(ostream& out, Film a){
	out<<"Film: "<<a.name<<" Year: "<<a.year<<" Director: "<<a.director<<" Country: "<<a.country;
	return out;
}

//####################################################################

struct Director 
{ 
	int id; 
	std::string name;
	Director(){}

	~Director(){
	}
	Director(int _id, std::string _name)
		:id(_id), name(_name){}
};


ostream& operator<<(ostream& out, Director a){
	out<<"Id: "<<a.id<<" Name: "<<a.name;
	return out;
}

//####################################################################
//####################################################################

struct Country
{
	int id;
	std::string name;
	Country(int _id, std::string _name)
		:id(_id), name(_name){}
};

ostream& operator<<(ostream& out, Country a){
	out<<"Id: "<<a.id<<" Name: "<<a.name;
	return out;
}

//####################################################################
//####################################################################

struct Genre
{
	int id;
	std::string genre;
	Genre(int _id, std::string _genre)
		:id(_id), genre(_genre){}
};

ostream& operator<<(ostream& out, Genre a){
	out<<"Id: "<<a.id<<" Genre: "<<a.genre;
	return out;
}

//####################################################################
//####################################################################

struct GenreF
{
	int film_id;
	int genre_id;
	GenreF(int _id,  int gid)
		:film_id(_id), genre_id(gid){}
};

ostream& operator<<(ostream& out, GenreF a){
	out<<"Film Id: "<<a.film_id<<" Genre Id: "<<a.genre_id;
	return out;
}	

//####################################################################
//####################################################################

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
		cout<<"File failed to open"<<endl;
	}
	std::string line;
	while(getline(file, line)){
		cbk(explode("#", line));
	}
}

//####################################################################
//####################################################################

int main(){
	std::string country("France");
	//*
	DBTable<Film> TBfilms;
	TBfilms.registerIndex("byYearName", DM::INDEX_TYPE::UNIQUE, &Film::year, &Film::name);

	//load data using rvalues
	fromFile("Data/films", [&TBfilms](std::vector<std::string> data)->void{
		static bool a=false;
		if(a && data.size()==7){
			TBfilms.insert(Film(stoi(data[0]), stoi(data[1]), data[2], stoi(data[3]), stoi(data[4]), stoi(data[5]), stoi(data[6])));   
		}
		a=true;
	});

	DBTable<Director*> TBdirectors;
	TBdirectors.registerIndex("byId", DM::INDEX_TYPE::UNIQUE, &Director::id);

		//load data using references
		fromFile("Data/directors", [&TBdirectors](std::vector<std::string> data)->void{
			 TBdirectors.insert(new Director(stoi(data[0]), data[1]));   
		});

		cout<<"==================== Join query =================\n";
		cout<<"Films between 1973 and 1975 :\n";

		Query<Film> queryFilms(&TBfilms);
		Query<Director*> queryDirectors(&TBdirectors);

		queryFilms.selectWhere(&Film::year, std::pair<int, int>(1973, 1975));
	try{
		int i=0;
		for(auto film : queryFilms.getResults()){
			cout<<"Film: "<<film->name<<" Year: "<<film->year;
			cout<<" Director: "<<queryDirectors.getUnique("byId", film->director)->name<<endl;

			if(++i>20){
				//break;
			}
		};
	}
	catch(const char* msg){
		cout<<msg<<endl;
	}

	//Directors that film in France in 1980 

	//load data for Countries
	DBTable<Country*> TBcountries;
	TBcountries.registerIndex("byId", DM::INDEX_TYPE::UNIQUE, &Country::id);
	TBcountries.registerIndex("byName", DM::INDEX_TYPE::UNIQUE, &Country::name);
	fromFile("Data/countries", [&TBcountries](std::vector<std::string> data)->void{
		TBcountries.insert(new Country(stoi(data[0]), data[1]));   
	});
	
	Query<Country*> queryCountries(&TBcountries);
 	//Directors that film in France in 1980 
 	try{
		queryFilms.selectWhere(&Film::year, {1980}).selectWhere(&Film::country, 
					{queryCountries.getUnique("byName", std::tuple<std::string>("France"))->id}); 

		std::cout<<"================= Directors that film in France in 1980 ===============\n";
		int j=0;

		for(auto film : queryFilms.getResults()){
			if(film){
			cout<<"Film: "<<film->name<<" Year: "<<film->year<<" Country: "<<film->country;
			cout<<" Director: "<<queryDirectors.getUnique("byId", film->director)->name<<endl;
			}
			if(++j>20){
				//break;
			}
		}
	}
	catch(const char* msg){
		cout<<msg<<endl;
	}

	DBTable<Genre> TBgenre;
	TBgenre.registerIndex("byGenre", DM::INDEX_TYPE::UNIQUE, &Genre::genre);
	
	//load data for Genres
	fromFile("Data/genres", [&TBgenre](std::vector<std::string> data)->void{
		if(data.size()==2){
			TBgenre.insert(Genre(stoi(data[0]), data[1]));
		}
	});
	
	TBgenre.registerIndex("byId", DM::INDEX_TYPE::UNIQUE, &Genre::id);

	//*
	DBTable<GenreF*> TBfilmG;
	//load data for Genres
	fromFile("Data/film_genres", [&TBfilmG](std::vector<std::string> data)->void{
		static bool a=false;
		if(a){
			TBfilmG.insert(new GenreF(stoi(data[0]), stoi(data[1])));
		}
		a=true;
	});

	TBfilmG.registerIndex("byFilmId", DM::INDEX_TYPE::NO_UNIQUE, &GenreF::film_id);
	TBfilmG.registerIndex("byGenreId", DM::INDEX_TYPE::NO_UNIQUE, &GenreF::genre_id);	

	// Select comedy film from Germany and Japan
	
	Query<GenreF*> queryFilmG(&TBfilmG);
	Query<Film> queryFilms2(&TBfilms);
	Query<Genre> queryGenre(&TBgenre);
	
	queryFilmG.selectWhere(&GenreF::genre_id, 
		queryGenre.selectWhere(&Genre::genre, {std::string("Comedy")}).getResults(&Genre::id)
	);
	queryFilms2.selectWhere(&Film::id, queryFilmG.getResults(&GenreF::film_id));
	queryFilms2.selectWhere(&Film::country, 
		queryCountries.selectWhere(&Country::name, 
			{std::string("Germany"), std::string("Japan")}).getResults(&Country::id)
	); 

	std::cout<<"================= Comedy films from Germany and from Japan (not join production) ===============\n";
	int k=0;
	for(auto& f : queryFilms2.getResults()){
		cout<<k++<<": "<<*f<<endl;
	}
	
	//###############################################################

	return 0;
}