/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 15:34:44 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/19 12:33:48 by ncasteln         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERVER_HPP__
# define __SERVER_HPP__

#include <iostream>
#include <vector>
#include <map>
#include "Location.hpp"

class Server
{
	public: // -------------------------------------------- PUBLIC CONSTRUCTORS
		Server( size_t );
		~Server( void );
		Server( const Server& );
	private:  // ----------------------------------------- PRIVATE CONSTRUCTORS
		Server( void );
		void operator=( const Server& );
		
	public: // ------------------------------------------------- PUBLIC MEMBERS
		size_t getIndex( void );
		std::map<std::string, std::string> getSettings( void );
		size_t setSettings( std::string key, std::string value );
	
	private: // ----------------------------------------------- PRIVATE MEMBERS
		size_t _i;
		bool is_default;
		std::map<std::string, std::string> _settings;
		std::vector<Location> location;
		
		// std::string				server_name;
		// std::string				listen;
		// std::string				root;
};

#endif /* __SERVER_HPP__ */
