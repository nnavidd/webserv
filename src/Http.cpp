/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:32:42 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/14 10:40:36 by ncasteln         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Http.hpp"

// ------------------------------------------------------------------- CANONICAL
Http::Http( void ) {};
Http::~Http( void ) {};
Http::Http( const Http& obj ) { (void)obj;/* Not implemented */};
void Http::operator=( const Http& rhs ) { (void)rhs;/* Not implemented */};

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
std::string& ltrim( std::string& s, const char* to_trim ) {
	size_t pos = s.find_first_not_of(to_trim);
	s.erase(0, pos);
	return (s);
}

std::string& rtrim( std::string& s, const char* to_trim ) {
	size_t pos = s.find_last_not_of(to_trim);
	s.erase(pos + 1);
	return (s);
}

std::string& trim( std::string& s, const char* to_trim ) {
	return (ltrim(rtrim(s, to_trim), to_trim));
}

static int parseHttp( std::string& line ) {
	std::string p1 = "keepalive_timeout";
	std::string p2 = "client_body_buffer_size";
	std::string dir = "server";
	
	if (line.compare(0, p1.length(), p1) == 0)
		std::cout << "**** PROP! ****" << std::endl;
	else if (line.compare(0, p2.length(), p2) == 0)
		std::cout << "**** PROP! ****" << std::endl;
	else if (line.compare(0, dir.length(), dir) == 0)
		std::cout << "**** SERVER! ****" << std::endl;
	
}

void Http::parseBlock(std::string& state, std::string& line, std::string nestedBlock) {
	if ((line.compare(0, 4, nestedBlock)) != 0) throw ParserExcept(E_NOHTTP);
	line.erase(0, 4);
	ltrim(line, SPACES);

	if (!OPENBLOCK(line[0])) throw ParserExcept(E_CONTEXTDECL);
	line.erase(0, 1);
	ltrim(line, SPACES);
	if (!COMMENT(line[0]) && !ENDVALUE(line[0])) {
		throw ParserExcept(E_CONTEXTDECL);	// http { helloworld
	}
	state = "HTTP";
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
			parseBlock(state, line, "http");
			continue ;
			// start finding HTTP
			// if ((line.compare(0, 4, "http")) != 0) throw ParserExcept(E_NOHTTP);
			// line.erase(0, 4);
			// ltrim(line, SPACES);

			// if (!OPENBLOCK(line[0])) throw ParserExcept(E_CONTEXTDECL);
			// line.erase(0, 1);
			// ltrim(line, SPACES);
			// if (COMMENT(line[0]) || ENDVALUE(line[0])) {
			// 	state = "HTTP";
			// 	continue ;
			// }
			// throw ParserExcept(E_CONTEXTDECL);	// http { helloworld
		}
		else if (state == "HTTP")
			if (parseHttp(line)) throw ParserExcept(E_INVPROP);
	}
}

void Http::addProp( std::string line ) {

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
	if (_n == E_CONTEXTDECL) return ("invalid context declaration");
	if (_n == E_INVPROP) return ("invalid property name");
	if (_n == E_ENDPROP) return ("syntax error, missing end of line `;`");
	return ("Unknow exception");
}
