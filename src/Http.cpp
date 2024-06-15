/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:32:42 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/15 17:02:36 by ncasteln         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Http.hpp"

// ------------------------------------------------------------------ CANONICAL
Http::Http( void ) {};
Http::~Http( void ) {};
Http::Http( const Http& obj ) { (void)obj;/* Not implemented */};
void Http::operator=( const Http& rhs ) { (void)rhs;/* Not implemented */};

// ---------------------------------------------------------- PARAM CONSTRUCTOR
Http::Http( int argc, char** argv ): 
	_n_server(0) {

	state.http = false; 
	state.server = -1;
	state.location = -1;
	
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
	
	std::string	line;
	parse(confFile, line);
	
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

std::string Http::getState( void ) {
	std::string lastState = "unset";
	if (state.http) lastState = "http";
	if (state.server > -1) lastState = "server";
	if (state.location > -1) lastState = "location";
	return (lastState);
}

void Http::parse( std::ifstream& confFile, std::string& line ) {
	while(getline(confFile, line, '\n')) {
		if (confFile.fail()) throw FileExcept(E_FAIL);

		trim(line, SPACES);										// line with only spaces
		if (line.empty()) continue ;							// empty lines
		if (COMMENT(line[0]) || ENDVALUE(line[0])) continue ;	// comment lines

		std::cout << "------------------------------------------------------------" << std::endl;
		std::cout << B("* [STATE] ") << "[" << getState() << "]" << std::endl;
		std::cout << G("* [LINE]  ") << "[" << line << "]" << std::endl;
	
		if (switchState(line)) {
			// if the state was switched NEED to look for {
			std::cout << R("* Switched state: ") << getState() << std::endl;
			std::cout << R("* Need still to consume: ") << "[" << line <<  "]" << std::endl;
		}
		// parseDirective ---> (line) { it will consume the rest of the line, or the new one }
		// Detect the context that need to be closed , using either a flag in state AND/OR the counter of the current level of brackets, always in the state
	}
}

bool Http::switchState( std::string& line ) {
	std::string http = "http";
	std::string server = "server";
	std::string location = "location";
	
	if (line.compare(0, http.length(), http) == 0) {
		line.erase(0, http.length()); 
		state.httpToOpen = true;
	}
	if (state.httpToOpen) {
		// if (state.http) throw ParserExcept(E_INVENTRY); // double http state	
		while (!line.empty()) {
			if (OPENBLOCK(line[0])) {
				state.httpToOpen = false;
				state.http = true;
				line.erase(0, 1);
				return (true);
			}
			line.erase(0, 1); // doesn't work !!! ------------- doesn't find something between CONTEXTNAME and { because it erases it 
		}
	}
	
	/* else */
	if (line.compare(0, server.length(), server) == 0) {
		line.erase(0, server.length());
		state.serverToOpen = true;
	}
	if (state.serverToOpen) {
		while (!line.empty()) {
			if (OPENBLOCK(line[0])) {
				state.serverToOpen = false;
				// check if the server opened is the same number of the last server added
				state.server = 1;
				line.erase(0, 1);
				return (true);
			}
			line.erase(0, 1);
		}
	}

	/* else  */
	if (line.compare(0, location.length(), location) == 0) {
		line.erase(0, location.length());
		state.locationToOpen = true;
	}
	if (state.locationToOpen) {
		while (!line.empty()) {
			if (OPENBLOCK(line[0])) {
				state.locationToOpen = false;
				// check if the server opened is the same number of the last server added
				state.location = 1;
				line.erase(0, 1);
				return (true);
			}
			line.erase(0, 1);
		}
	}
	return (false);
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
	if (_n == E_INVENTRY) return ("invalid dirctive or context name");
	if (_n == E_CONTEXTDECL) return ("invalid context declaration");
	if (_n == E_INVPROP) return ("invalid property name");
	if (_n == E_SYNTAX) return ("syntax error");
	if (_n == E_ENDPROP) return ("syntax error, missing end of line `;`");
	return ("Unknow exception");
}

// -------------------------------------------------------------------- STATICS
const std::string Http::_httpDirList[2] = {
	"keepalive_timeout",
	"client_body_buffer_size" };
const std::string Http::_serverDirList[3] = {
	"server_name",
	"listen",
	"root" };
const std::string Http::_locationDirList[5] = {
	"root", 
	"index", 
	"method", 
	"autoindex", 
	"cgi" };
