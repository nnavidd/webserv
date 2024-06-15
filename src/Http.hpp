/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:32:42 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/15 12:43:37 by ncasteln         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __HTTP_HPP__
# define __HTTP_HPP__

#include "Server.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <map>

#include <sys/stat.h>	// stat()
#include <cctype>		// isspace
#include "colors.h"

#define	SPACES			" \t\v\f\r"
#define	NEWLINE(c)		((c) == '\n')
#define	COMMENT(c)		((c) == '#')
#define	ENDVALUE(c)		((c) == ';')
#define	OPENBLOCK(c)	((c) == '{')
#define	CLOSEBLOCK(c)	((c) == '}')

enum file_err {
	E_TOOARGS,
	E_INVFILE,
	E_ISDIR,
	E_FAIL
};

enum parser_err {
	E_INVENTRY,
	E_CONTEXTDECL,
	E_SYNTAX,
	E_INVPROP,
	E_ENDPROP
};

struct parsingState {
	bool beginOpen;
	bool httpOpen;
	bool serverOpen;
	bool locationOpen;
};

class Http
{
	public: // -------------------------------------------- PUBLIC CONSTRUCTORS
		Http( int argc, char** argv );
		~Http( void );
	private:  // ----------------------------------------- PRIVATE CONSTRUCTORS
		Http( void );
		Http( const Http& );
		void operator=( const Http& );
		
	public: // ------------------------------------------------- PUBLIC MEMBERS
	
	private: // ----------------------------------------------- PRIVATE MEMBERS
		std::map<std::string, std::string> _directive;
		
		size_t _n_server;
		std::vector<Server> _server;

		bool isDirectory( char* path ); // ---------------------------- PARSING
		void parse( std::ifstream& confFile );
		void parseContext( std::string& line, std::string& currContext );
		bool parseDirectives( std::string& line, std::string& currBlock );
		void addDirective( std::string& line, std::string& currBlock, const std::string dir );

		struct parsingState state;
		void openState( std::string& currContext, std::string nextContext );
		void closeState( std::string context );

		static const std::string _httpDirList[2]; // ---------- DIRECTIVE LISTS
		static const std::string _serverDirList[3];
		static const std::string _locationDirList[5];

		class FileExcept; // --------------------------------------- EXCEPTIONS
		class ParserExcept;
};

class Http::FileExcept: public std::exception
{
	private:
		int	_n;
	public:
		FileExcept( file_err n );
		const char* what() const throw();
};

class Http::ParserExcept: public std::exception
{
	private:
		int	_n;
	public:
		ParserExcept( parser_err n );
		const char* what() const throw();
};


#endif /* __HTTP_HPP__ */
