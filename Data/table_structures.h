#ifndef TABLE_STRUCTURES_H
#define TABLE_STRUCTURES_H

#include <iostream>
#include <string>

using namespace std;

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

#endif
