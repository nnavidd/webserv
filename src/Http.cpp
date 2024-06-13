/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:32:42 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/13 14:43:48 by ncasteln         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Http.hpp"

// ------------------------------------------------------------------- CANONICAL
Http::Http( void ) {};
Http::~Http( void ) {};
Http::Http( const Http& obj ) {/* Not implemented */};
Http& Http::operator=( const Http& rhs ) {/* Not implemented */};

// ---------------------------------------------------------- PARAM CONSTRUCTOR
Http::Http( int argc, char** argv ) {
	if (argc > 2) throw InvalidConf(E_TOOARGS);
	std::ifstream confFile;
	if (!argv[1] || std::string(argv[1]).empty())
		confFile.open("./conf/default.conf", std::ifstream::in);
	else {
		confFile.open(argv[1], std::ifstream::in);
		if (!confFile) throw InvalidConf(E_INVFILE);
		if (isDirectory(argv[1])) throw InvalidConf(E_ISDIR);
	}
	if (!confFile) throw InvalidConf(E_INVFILE);
	parse(confFile);
	confFile.close();
};

bool Http::isDirectory( char* path ) {
	struct stat s;
	if (stat(path, &s) == 0)
		if (S_ISDIR(s.st_mode)) return (true);
	else
		return (true);
	return (false);
}

// -------------------------------------------------------------------- PARSING
static bool jumpSpaces( std::string::iterator& it ) {
	while (std::isspace(*it)) {
		std::cout << "[SP]" << std::endl;
		it++;
	}
}

void Http::parse( std::ifstream& confFile ) {
	std::string	line;
	std::string::iterator it;
	std::string state = "HTTP"; // can change to SERV or LOC

	while(getline(confFile, line)) {
		if (confFile.fail()) throw InvalidConf(E_FAIL);
		if (line.empty()) continue ;

		std::cout << "LINE: [" << line << "]" << std::endl;
		// here goes parse
	}
}

// ------------------------------------------------------------ FILE EXCEPTIONS
Http::InvalidConf::InvalidConf( file_err n ): _n(n) {};

const char* Http::InvalidConf::what() const throw() {
	if (_n == E_TOOARGS) return ("too many args");
	if (_n == E_INVFILE) return ("invalid file");
	if (_n == E_ISDIR) return ("arg provided is a directory");
	if (_n == E_FAIL) return ("fail reading the file");
	return ("Unknow exception");
}

// ----------------------------------------------------------- PARSE EXCEPTIONS
Http::ParseExcep::ParseExcep( parse_err n ): _n(n) {};

const char* Http::ParseExcep::what() const throw() {

	return ("Unknow exception");
}
