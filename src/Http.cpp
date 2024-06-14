/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:32:42 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/14 09:22:11 by ncasteln         ###   ########.fr       */
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
	if (argc > 2) throw FileExcept(E_TOOARGS);
	std::ifstream confFile;
	if (!argv[1] || std::string(argv[1]).empty())
		confFile.open("./conf/default.conf", std::ifstream::in);
	else {
		confFile.open(argv[1], std::ifstream::in);
		if (!confFile) throw FileExcept(E_INVFILE);
		if (isDirectory(argv[1])) throw FileExcept(E_ISDIR);
	}
	if (!confFile) throw FileExcept(E_INVFILE);
	parse(confFile);
	confFile.close();
};

bool Http::isDirectory( char* path ) {
	struct stat s;
	if (stat(path, &s) == 0) {
		if (S_ISDIR(s.st_mode))
			return (true);
	}
	else
		return (true);
	return (false);
}

// -------------------------------------------------------------------- PARSING
static std::string& ltrim( std::string& s, const char* to_trim ) {
	size_t pos = s.find_first_not_of(to_trim);
	s.erase(0, pos);
	return (s);
}

static std::string& rtrim( std::string& s, const char* to_trim ) {
	size_t pos = s.find_last_not_of(to_trim);
	s.erase(pos + 1);
	return (s);
}

static std::string& trim( std::string& s, const char* to_trim ) {
	return (ltrim(rtrim(s, to_trim), to_trim));
}

static bool isComment( char c ) {
	if (c == '#' || c == ';')
		return (true);
	return (false);
}

void Http::parse( std::ifstream& confFile ) {
	std::string	line;

	std::string state = "BEGIN"; // can change to SERV or LOC
	while(getline(confFile, line)) {
		if (confFile.fail()) throw FileExcept(E_FAIL);
		trim(line, SPACES);										// line with only spaces
		if (line.empty()) continue ;							// empty lines
		if (COMMENT(line[0]) || ENDVALUE(line[0])) continue ;	// comment lines

		std::cout << "------------------------------------------------------------------" << std::endl;
		std::cout << B("STATE:	") << state << std::endl;
		std::cout << B("LINE:	[") << line << B("]") << std::endl;
		
		// states
		if (state == "BEGIN") {
			// start finding HTTP
			if ((line.substr(0, 4)) != "http") throw ParserExcept(E_NOHTTP);
			line.erase(0, 4);
			trim(line, SPACES);
			if (line[0] != '{' && *line.end() != '{') throw ParserExcept(E_BLOCKOPEN);
			
			// change state
			state = "HTTP";
			continue ;
		}
		else if (state == "HTTP") {			
			// extract prop
			if (line.find_first_of(SPACES) == std::string::npos)
				throw ParserExcept(E_INVPROP); // means there is no space after prop name
			addProp(line);
			continue ;
		}
	}
}

void Http::addProp( std::string line ) {
	size_t endPropName = line.find_first_of(SPACES);
	std::cout << "first of spaces is: " << endPropName << std::endl;
	if (line.substr(0, endPropName) == "keepalive_timeout") {
		line.erase(0, endPropName);
		trim(line, SPACES);
		if (line.find(';') == std::string::npos) throw ParserExcept(E_ENDPROP);
		keepalive_timeout = line.substr(0, line.find(';'));
		std::cout << G("* PROPERTY ADDED ---> keepalive_timeout = ") << keepalive_timeout << std::endl;
		return ;
	}
	else if (line.substr(0, endPropName) == "client_body_buffer_size") {
		line.erase(0, endPropName);
		trim(line, SPACES);
		if (line.find(';') == std::string::npos) throw ParserExcept(E_ENDPROP);
		client_body_buffer_size = line.substr(0, line.find(';'));
		std::cout << G("* PROPERTY ADDED ---> client_body_buffer_size = ") << client_body_buffer_size << std::endl;
		return ;
	}
	throw ParserExcept(E_INVPROP);
}

// ------------------------------------------------------------ FILE EXCEPTIONS
Http::FileExcept::FileExcept( file_err n ): _n(n) {};

const char* Http::FileExcept::what() const throw() {
	if (_n == E_TOOARGS) return ("too many args");
	if (_n == E_INVFILE) return ("invalid file");
	if (_n == E_ISDIR) return ("arg provided is a directory");
	if (_n == E_FAIL) return ("fail reading the file");
	return ("Unknow exception");
}

// ----------------------------------------------------------- PARSE EXCEPTIONS
Http::ParserExcept::ParserExcept( parser_err n ): _n(n) {};

const char* Http::ParserExcept::what() const throw() {
	if (_n == E_NOHTTP) return ("no http block");
	if (_n == E_BLOCKOPEN) return ("syntax error, missing separator `{`");
	if (_n == E_INVPROP) return ("invalid property name");
	if (_n == E_ENDPROP) return ("syntax error, missing end of line `;`");
	return ("Unknow exception");
}
