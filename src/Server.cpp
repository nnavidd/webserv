/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:32:42 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/15 11:39:24 by ncasteln         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

// ------------------------------------------------------------------ CANONICAL
Server::Server ( void ) {};
Server::~Server( void ) {};
Server::Server( const Server& obj ) { (void)obj;/* Not implemented */};
void Server::operator=( const Server& rhs ) { (void)rhs;/* Not implemented */};

// ---------------------------------------------------------- PARAM CONSTRUCTOR
Server::Server ( size_t i ): _i(i) {
	
};

// ------------------------------------------------------------- PUBLIC MEMBERS
size_t Server::getIndex( void ) { return (_i); };

size_t Server::setSettings( std::string key, std::string value ) {
	_settings[key] = value;
};
