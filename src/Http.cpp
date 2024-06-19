/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:32:42 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/19 12:41:04 by ncasteln         ###   ########.fr       */
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
	_n_server(0),
	_currLvl(HTTP),
	_activeContext(HTTP) {

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

// ---------------------------------------------------------------------- UTILS
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

// -------------------------------------------------------------------- PARSING
void Http::parse( std::ifstream& confFile ) {	
	std::string line;
	std::string directive;
	std::string value;

	while(getline(confFile, line, '\n')) {
		if (confFile.fail()) throw FileExcept(E_FAIL);
		
		if (line.empty()) continue ;							// empty lines are jumped
		if (COMMENT(line[0]) || ENDVALUE(line[0])) continue ;	// comment lines are jumped

		// store the prev level
		setCurrIndentation(line); // set curr level based on line TABS
		std::cout << "------------------------------------------------------------" << std::endl;
		std::cout << G("* [LINE]     ") << "[" << line << "]" << std::endl;
		std::cout << B("* [CURR IND] ") << "[ " << displayIndentantion(_currLvl) << " ]" << std::endl;

		directive = getDirective(line);
		if (!isValidDirective(directive))						// the directive exists
			throw ParserExcept(E_INVDIR);
		if (openContext(directive)) {
			// std::cout << R("* Created server NUMBER: ") << _n_server << std::endl;
			// Server s(_n_server);
			// _server.push_back(s);
			// _n_server++;
			continue ;
		}
		closeContext();												// if the current level of indentation is lower than the one before, one or two context are closed 
		if (!isCorrectContextOpen())			// check if the context for {{{THAT}}} directive is open
			throw ParserExcept(E_INVCONTEXT);
		value = getValue(directive, line);
		storeDirective(directive, value);
	}
	// displayConfiguration();
}

/*	Get the number of tabs preceding the line and erase them. */
void Http::setCurrIndentation( std::string& line ) {
	int i = 0;
	while (line[static_cast<size_t>(i)] == '\t')
		i++;
	line.erase(0, static_cast<size_t>(i));
	if (i == 0) _currLvl = HTTP;
	if (i == 1) _currLvl = SERVER;
	if (i == 2) _currLvl = LOCATION;
	if (i > 2) throw ParserExcept(E_INVIND);
}

std::string Http::displayIndentantion( indentation ind ) {
	if (ind == HTTP) return ("http");
	if (ind == SERVER) return ("server");
	if (ind == LOCATION) return ("location");
	return ("init");
}

std::string Http::getDirective( std::string& line ) {
	if (line.empty())
		throw ParserExcept(E_ONLYTABS);
	size_t firstSpace = line.find_first_of(SPACES);				// go to first space
	// if not exist throw error ---- NO because already checked in isValidDirective
	std::string directive = line.substr(0, firstSpace);		// cut the string
	return (directive); // can be also a context names
}

bool Http::isValidDirective( std::string Directive ) {	
	size_t size;
	const std::string* list;

	if (_currLvl == HTTP) {
		list = _httpDirectives;
		size = N_HTTP_DIR;
	}
	if (_currLvl == SERVER) {
		list = _serverDirectives;
		size = N_SERVER_DIR;
	}
	if (_currLvl == LOCATION) {
		list = _locationDirectives;
		size = N_LOCATION_DIR;
	}
	for (size_t i = 0; i < size; i++) {
		if (list[i] == Directive)
			return (true);
	}
	return (false);
}

bool Http::openContext( std::string Directive ) {
	if (Directive == "server") {
		_activeContext = SERVER;
		std::cout << G("* [ACTIVATED CONTEXT]: ") << displayIndentantion(_activeContext) << std::endl;
		return (true);
	} 
	if (Directive == "location") {
		_activeContext = LOCATION;
		std::cout << G("* [ACTIVATED CONTEXT]: ") << displayIndentantion(_activeContext) << std::endl;
		return (true);
	}
	return (false);
}

/*	If the currLvl of indentation is lower than the one of the active context,
	the active context is closed and becomes the one of the currLvl.
*/
void Http::closeContext( void ) {
	if (_currLvl < _activeContext) {
		std::cout << R("* [CLOSED CONTEXT]: ") << displayIndentantion(_activeContext) << std::endl;
		_activeContext = _currLvl;
		std::cout << G("* [ACTIVE CONTEXT]: ") << displayIndentantion(_activeContext) << std::endl;
	}
}

/*	The active context and the current level of indentation has to be always
	equal, otherwise it means, that a directive was written in the wrong one.
*/
bool Http::isCorrectContextOpen( void ) {
	if (_activeContext != _currLvl)
		return (false);
	return (true);
}

std::string Http::getValue( std::string directive, std::string& line ) {
	size_t end_of_decl;
	std::string value;
	
	line.erase(0, directive.length());
	ltrim(line, SPACES);
	end_of_decl = line.find(';');
	if (end_of_decl == std::string::npos)
		throw ParserExcept(E_ENDDECL);
	value = line.substr(0, end_of_decl);
	rtrim(value, SPACES);
	return (value);
}

void Http::storeDirective(std::string directive, std::string value) {
	// std::cout <<"* SETTING: ";
	// if (_activeContext == HTTP) {
	// 	std::cout << directive << " - " << value << std::endl;
	// 	_settings[directive] = value;
	// }
	// if (_activeContext == SERVER) {
	// 	std::cout << directive << " - " << value << std::endl;
	// 	std::cout << "	* SERVER NUMBER: " << _server.back().getIndex() << std::endl;
	// 	_server.back().setSettings(directive, value);
	// }
	// if (_activeContext == LOCATION) {
	// 	std::cout << directive << " - " << value << std::endl;
	// 	// implement
	// }
	
}

// -------------------------------------------------------------------- DISPLAY
void Http::displayServerConf( void ) {
	std::cout << G("[SERVER]");
	std::vector<Server>::iterator currServ = _server.begin();
	while (currServ != _server.end()) {
		std::map<std::string, std::string> settings = (*currServ).getSettings();
		std::map<std::string, std::string>::iterator currSetting = settings.begin();
		while (currSetting != settings.end()) {
			std::cout << "  * " << (*currSetting).first << ":	" << (*currSetting).second << std::endl;
			currSetting++;	
		}
		currServ++;
	}
}

void Http::displayConfiguration( void ) {
	std::cout << B("*** WEB SERVER CONFIGURATION ***") << std::endl;
	
	std::cout << G("[HTTP]") << std::endl;
	std::map<std::string, std::string>::iterator it = _settings.begin();
	while (it != _settings.end()) {
		std::cout << "  * " << (*it).first << ":	" << (*it).second << std::endl;
		it++;
	}
	displayServerConf();
	std::cout << G("[LOCATION]") << std::endl;
}


// ------------------------------------------------------------ FILE EXCEPTIONS
Http::FileExcept::FileExcept( file_err n ): _n(n) {};
const char* Http::FileExcept::what() const throw() {
	if (_n == E_TOOARGS) return ("too many args");
	if (_n == E_INVFILE) return ("invalid file");
	if (_n == E_ISDIR) return ("arg provided is a directory");
	if (_n == E_FAIL) return ("fail reading the file");
	return ("Unknow File exception");
}

// ----------------------------------------------------------- PARSE EXCEPTIONS
Http::ParserExcept::ParserExcept( parser_err n ): _n(n) {};
const char* Http::ParserExcept::what() const throw() {
	std::string base = "invalid configuration: ";
	if (_n == E_INVDIR) base += "directive does't exist";
	if (_n == E_ONLYTABS) base += "directive expected after tabs";
	if (_n == E_INVCONTEXT) base += "directive declared in wrong context";
	if (_n == E_INVIND) base += "invalid indentation";
	if (_n == E_ENDDECL) base += "directive end not found";
	return (base.c_str());
}

// -------------------------------------------------------------------- STATICS
const std::string Http::_httpDirectives[N_HTTP_DIR] = {
	"keepalive_timeout",
	"client_body_buffer_size",
	"server" };
const std::string Http::_serverDirectives[N_SERVER_DIR] = {
	"server_name",
	"listen",
	"root",
	"location" };
const std::string Http::_locationDirectives[N_LOCATION_DIR] = {
	"uri", 
	"root", 
	"index", 
	"method", 
	"autoindex", 
	"cgi" };
