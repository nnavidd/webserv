/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:32:42 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/15 12:44:22 by ncasteln         ###   ########.fr       */
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

	state.beginOpen = false;
	state.httpOpen = false; // move from here, clean the function
	state.serverOpen = false;
	state.locationOpen = false;
	
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

void Http::addDirective( std::string& line, std::string& currContext, const std::string key ) {
	std::string value;

	line.erase(0, key.length());
	if (line[0] != ' ' && line[0] != '\t') // no separator!
		throw ParserExcept(E_SYNTAX);
	ltrim(line, SPACES);
	if (line.find(';') == std::string::npos ) // got to the end of the value
		throw ParserExcept(E_ENDPROP);
	value = line.substr(0, line.find(';'));
	trim(value, SPACES);

	if (currContext == "http")
		_directive[key] = value;
	
	if (currContext == "server")
		_server.back().setSettings(key, value);
	std::cout << G("* [ ") << currContext << G(" DIRECTIVE ADDED ] ---> ") << key << " = " << value << std::endl;
}

/*	Generic function to parse all the directives of any context. 
	@param n: number of possible directives of that type
	@param dir: pointer used to correct point to the list of directives
*/
bool Http::parseDirectives( std::string& line, std::string& currContext ) {
	size_t n;
	const std::string* dirList;
	std::string key;
	
	if (currContext == "http") {
		dirList = Http::_httpDirList;
		n = 2;
	}
	if (currContext == "server") {
		dirList = Http::_serverDirList;
		n = 5;
	}
	for (size_t i = 0; i < n; i++) {
		key = dirList[i];
		if (line.compare(0, key.length(), key) == 0) {
			addDirective(line, currContext, key);
			return (true);
		}
	}
	return (false);
}

/*	Tries to:
	- Look for the directives of the currentBlock
	- If the line has not one of those directives, it looks for the next block
	- Important to note, the location has a different syntax like "location /hello {...}" but maybe can be simplified and put the prop inside it 
*/
void Http::parseContext( std::string& line, std::string& currContext ) {
	std::string nextContext;
	if (currContext == "begin") nextContext = "http";
	else if (currContext == "http") nextContext = "server";
	else if (currContext == "server") nextContext = "location";
	else if (currContext == "location") nextContext = "end";

	if (CLOSEBLOCK(line[0])) {
		line.erase(0, 1);
		ltrim(line, SPACES);
		if (!COMMENT(line[0]) && !ENDVALUE(line[0]) && !line.empty()) // FIXED: example [server { ok] OR [server { #ok] 
			throw ParserExcept(E_CONTEXTDECL);
		closeState(currContext);
		return ;
	}

	// check if valid context name
	if ((line.compare(0, nextContext.length(), nextContext)) != 0)
		throw ParserExcept(E_INVENTRY); // here should catch the invlid property too
	line.erase(0, nextContext.length());
	ltrim(line, SPACES);

	// check if valid opening bracket
	if (!OPENBLOCK(line[0])) 
		throw ParserExcept(E_CONTEXTDECL);
	line.erase(0, 1);
	ltrim(line, SPACES);
	
	// check if comment or end value
	if (!COMMENT(line[0]) && !ENDVALUE(line[0]) && !line.empty()) // FIXED: example [server { ok] OR [server { #ok]
		throw ParserExcept(E_CONTEXTDECL);
	
	// arriving here means open the context
	openState(currContext, nextContext);
}

void Http::openState( std::string& currContext, std::string nextContext ) {
	if (currContext == "begin")
		state.httpOpen = true;
	else if (currContext == "http")
		state.serverOpen = true;
	else if (currContext == "server")
		state.locationOpen = true;
	else if (currContext == "location")
		throw ParserExcept(E_INVENTRY);
	currContext = nextContext;
	std::cout << R("* Context open: ") << currContext << std::endl;
}
void Http::closeState( std::string currContext ) {
	std::cout << R("* Context close: ") << currContext << std::endl;
	if (currContext == "http") state.httpOpen = false;
	else if (currContext == "server") state.serverOpen = false;
	else if (currContext == "location") state.locationOpen = false;
}

void Http::parse( std::ifstream& confFile ) {
	std::string	line;
	std::string currContext = "begin"; // can change to SERV or LOC

	while(getline(confFile, line)) {
		if (confFile.fail()) throw FileExcept(E_FAIL);
		
		trim(line, SPACES);										// line with only spaces
		if (line.empty()) continue ;							// empty lines
		if (COMMENT(line[0]) || ENDVALUE(line[0])) continue ;	// comment lines
		std::cout << "------------------------------------------------------------------" << std::endl;
		std::cout << B("* STATE: [") << currContext << B("]") << std::endl;
		std::cout << B("* LINE:  [") << line << B("]") << std::endl;

		// states
		if (currContext == "begin") {
			parseContext(line, currContext); continue ;
		}
		else if (currContext == "http") {
			if (parseDirectives(line, currContext)) continue ;
			parseContext(line, currContext);
		}
		else if (currContext == "server") {
			if (state.serverOpen) { // create a new server ONLY IF the precedent is closed
				Server s(_n_server);
				_server.push_back(s);
				_n_server++;
			}
			if (parseDirectives(line, currContext)) continue ; // either OPEN or NEW server, the directives are parsed
			parseContext(line, currContext);
		}
	}
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
