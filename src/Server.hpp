/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 15:34:44 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/13 14:32:04 by ncasteln         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERVER_HPP__
# define __SERVER_HPP__

#include <vector>
#include "Location.hpp"

class Server
{
	public:
		Server( void );
		~Server( void );

	private:
		bool 					is_default;
		std::string				server_name;
		std::string				listen;
		std::string				root;
		std::vector<Location>	location;
	public:
		static const std::string keepalive_timeout;
		static const std::string client_body_buffer_size;
};

#endif /* __SERVER_HPP__ */
