#include <iostream>
#include <vector>
#include <iomanip>

#include "DBtable.h"
#include "query.h"
#include "Data/table_structures.h"

using namespace std;
using namespace DM;

//####################################################################
//####################################################################
//####################################################################
//####################################################################

int main(){

	DBTable<Film> TBfilms;
	TBfilms.registerIndex("byNameYear", DM::INDEX_TYPE::UNIQUE, &Film::name, &Film::year);
	TBfilms.registerIndex("byYearDirectorCountry", 
			DM::INDEX_TYPE::NO_UNIQUE, 
			&Film::year, 
			&Film::director, 
			&Film::country);

	fromFile("Data/films", [&TBfilms](std::vector<std::string> data)->void{
		static bool a=false;
		if(a && data.size()==7){
			TBfilms.emplace(stoi(data[0]), stoi(data[1]), data[2], stoi(data[3]), stoi(data[4]), stoi(data[5]), stoi(data[6]));   
		}
		a=true;
	});
	
	DBTable<Director> TBdirectors;
	TBdirectors.registerIndex("byId", DM::INDEX_TYPE::UNIQUE, &Director::id);
	fromFile("Data/directors", [&TBdirectors](std::vector<std::string> data)->void{
		 TBdirectors.emplace(stoi(data[0]), data[1]);
	});




	//*
	cout<<"==================== Join query =================\n";
	cout<<"Films between 1973 and 1975 by Karlson, Phil:\n";

	Query<Film> queryFilms(&TBfilms);
	Query<Director> queryDirectors(&TBdirectors);
	queryFilms.selectWhere(&Film::year, 1973, 1975);
	queryDirectors.selectWhere(&Director::name, {std::string("Karlson, Phil")});
	auto results=queryFilms.innerJoin(&queryDirectors, &Director::id, &Film::director);	
	try{
		int i=0;
		for(auto tp : results){
			cout<<"Film: "<<std::get<0>(tp)->name;//<<" Year: "<<film->year;
			cout<<" Director: "<<std::get<1>(tp)->name<<endl;
		};
	}
	catch(const char* msg){
		cout<<msg<<endl;
	}

	//load data for Countries
	DBTable<Country> TBcountries;
	TBcountries.registerIndex("byId", DM::INDEX_TYPE::UNIQUE, &Country::id);
	TBcountries.registerIndex("byName", DM::INDEX_TYPE::UNIQUE, &Country::name);
	fromFile("Data/countries", [&TBcountries](std::vector<std::string> data)->void{
		TBcountries.emplace(stoi(data[0]), data[1]);   
	});

	Query<Country> queryCountries(&TBcountries);

 	try{
		queryCountries.selectWhereNot(&Country::name, {std::string("Peru"), std::string("France")}); 
		for(auto country : queryCountries.getResults()){
			cout<<"Country: "<<country->name<<endl;
		};
	}
	catch(const char* msg){
		cout<<msg<<endl;
	}

	std::cout<<"\n===========================================\n";
	std::cout<<"Directors that filmed in Russia:\n";

	queryCountries.selectWhere(&Country::name, {std::string("Russia")});
	auto dr=queryDirectors.innerJoin(
		queryFilms.innerJoin(&queryCountries, &Country::id, &Film::country),
		&Film::director, &Director::id
	);
	
	for(auto rst : dr){
		cout<<"Director: "<<std::get<0>(rst)->name<<endl;
	};
	
	std::cout<<"\n===========================================\n";
	std::cout<<"******* Films between 1973 and 1975 *******\n";

	queryFilms.selectWhere(&Film::year, 1973, 1975);
	auto countryFilm=queryFilms.innerJoin(&queryCountries, &Country::id, &Film::country);
	auto resultsA=queryDirectors.innerJoin(countryFilm, &Film::director, &Director::id);

	try{
		cout<<left<<setw(14)<<"Country";
		cout<<setw(6)<<"Year";
		cout<<left<<setw(30)<<"Director: ";
		cout<<"Film\n";
		
		for(auto film : resultsA){
			cout<<left<<setw(14)<<std::get<2>(film)->name;
			cout<<setw(6)<<std::get<1>(film)->year;
			cout<<left<<setw(30)<<std::get<0>(film)->name;
			cout<<std::get<1>(film)->name<<"\n";
		};
	}
	catch(const char* msg){
		cout<<msg<<endl;
	}

	DBTable<Genre> TBgenre;
	TBgenre.registerIndex("byGenre", DM::INDEX_TYPE::UNIQUE, &Genre::genre);
	TBgenre.registerIndex("byId", DM::INDEX_TYPE::UNIQUE, &Genre::id);
	//load data for Genres
	fromFile("Data/genres", [&TBgenre](std::vector<std::string> data)->void{
		if(data.size()>1){
			TBgenre.emplace(stoi(data[0]), std::string(data[1]));
		}
	});

	//load data for Genres
	DBTable<GenreF> TBgenreF;
	fromFile("Data/film_genres", [&TBgenreF](std::vector<std::string> data)->void{
		static bool a=false;
		if(a){
			TBgenreF.emplace(stoi(data[0]), stoi(data[1]));
		}
		a=true;
	});
	TBgenreF.registerIndex("byFilmId", DM::INDEX_TYPE::NO_UNIQUE, &GenreF::film_id);
	TBgenreF.registerIndex("byGenreId", DM::INDEX_TYPE::NO_UNIQUE, &GenreF::genre_id);	

	// Select comedy film from Germany
	Query<GenreF> queryGenreF(&TBgenreF);
	Query<Film> queryFilms2(&TBfilms);
	Query<Genre> queryGenre(&TBgenre);
	
	queryCountries.selectWhere(&Country::name, {std::string("Germany"), std::string("Japan"), std::string("Canada")});

	auto results2=queryFilms2.innerJoin(&queryCountries, &Country::id, &Film::country);
	auto results3=queryGenreF.innerJoin(results2, &Film::id, &GenreF::film_id);

	queryGenre.selectWhere(&Genre::genre, {std::string("Comedy")});
	auto results4=queryGenre.innerJoin(results3, &GenreF::genre_id, &Genre::id);

	std::cout<<"*** Comedy films from Canada, Germany and from Japan (not join production) ***\n";

	for(auto& tp : results4){
		cout<<" "<<std::get<2>(tp)->year;
		cout<<" "<<std::get<3>(tp)->name;	
		cout<<" "<<std::get<0>(tp)->genre;
		cout<<" "<<std::get<2>(tp)->name<<std::endl;
	}	
	// */

	//###############################################################
	//###############################################################
	/*
	queryCountries.selectWhere(&Country::name, {std::string("Germany"), std::string("Japan"), std::string("Canada")});
	queryGenre.selectWhere(&Genre::genre, {std::string("Comedy")});

	auto result4=queryGenre.innerJoin(
		queryGenreF.innerJoin(
			queryFilms2.innerJoin(
				&queryCountries, 
				&Country::id, &Film::country), 
			&Film::id, &GenreF::film_id), 
		&GenreF::genre_id, &Genre::id);

	////////////////////////////////////////////
	queryCountries.selectWhere(&Country::name, {std::string("Germany"), std::string("Japan"), std::string("Canada")});
	queryGenre.selectWhere(&Genre::genre, {std::string("Comedy")});
	auto results4=queryGenre.innerJoin(
		queryGenreF.innerJoin(
			queryFilms2.innerJoin(&queryCountries, &Country::id, &Film::country), 	
			&Film::id, &GenreF::film_id
		), 
		&GenreF::genre_id, &Genre::id
	);

	////////////////////////////////////////////

	 * */

	//###############################################################

	return 0;
}
