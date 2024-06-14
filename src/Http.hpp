/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:32:42 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/14 09:19:11 by ncasteln         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __HTTP_HPP__
# define __HTTP_HPP__

#include "Server.hpp"
#include <stdexcept>
#include <fstream>
#include <iostream>

#include <sys/stat.h>	// stat()
#include <cctype>		// isspace
#include "colors.h"

#define	SPACES		" \t\v\f\r"
#define	COMMENT(c)	((c) == '#')
#define	ENDVALUE(c)	((c) == ';')

enum file_err {
	E_TOOARGS,
	E_INVFILE,
	E_ISDIR,
	E_FAIL
};

enum parser_err {
	E_NOHTTP,
	E_BLOCKOPEN,
	E_INVPROP,
	E_ENDPROP,
};

class Http
{
	public:
		Http( void ); // -------------------------------------------- CANONICAL
		~Http( void );
		Http( const Http& );
		Http& operator=( const Http& );
		Http( int argc, char** argv );  // ---------------------------- PARAM CONSTRUCTOR
	private:
		std::string keepalive_timeout;
		std::string client_body_buffer_size;
		std::vector<Server> server;

		bool isDirectory( char* path ); // --------------------- PARSING
		void parse( std::ifstream& confFile );
		void addProp( std::string propName );

		class FileExcept; // ------------------------------------- EXCEPTIONS
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
