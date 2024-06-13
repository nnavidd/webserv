/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Http.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:32:42 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/13 14:43:38 by ncasteln         ###   ########.fr       */
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

enum file_err {
	E_TOOARGS,
	E_INVFILE,
	E_ISDIR,
	E_FAIL
};

enum parse_err {
	E_SOMETHING,
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
		std::vector<Server> server;

		bool isDirectory( char* path ); // --------------------- PARSING
		void parse( std::ifstream& confFile );

		class InvalidConf; // ------------------------------------- EXCEPTIONS
		class ParseExcep;
		file_err _fe;  // ------------------------------------- ERR LISTS
		parse_err _pe;
};

class Http::InvalidConf: public std::exception
{
	private:
		int	_n;
	public:
		InvalidConf( file_err n );
		const char* what() const throw();
};

class Http::ParseExcep: public std::exception
{
	private:
		int	_n;
	public:
		ParseExcep( parse_err n );
		const char* what() const throw();
};


#endif /* __HTTP_HPP__ */
