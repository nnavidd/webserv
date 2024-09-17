/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConf.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nico <nico@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/15 12:02:11 by ncasteln          #+#    #+#             */
/*   Updated: 2024/09/09 13:57:32 by nico             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERVERCONF_HPP__
# define __SERVERCONF_HPP__

#include "AConf.hpp"
#include "LocationConf.hpp"

class ServerConf: public AConf
{
	public:
		ServerConf( std::map<std::string, std::string> settings );
		~ServerConf( void );
		ServerConf& operator=( const ServerConf& );

		void setDefaults( void );
		void setSetting( std::string key, std::string value, context active );
		void addNestedBlock( context type );
		std::string getPort( void );
		const std::vector<LocationConf>& getLocation( void ) const;

		enum conf_err checkSettings( void );

		void displaySettings( void ) const;
		static const std::string serverSettings[N_SERVER_DIR];

	private:
		std::vector<LocationConf> _location;

		// UN-USED CONSTRUCTORS
		ServerConf( void );
};

#endif /* __SERVERCONF_HPP__ */
