/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ncasteln <ncasteln@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:24:40 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/20 15:06:14 by ncasteln         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.h"

int main ( int argc, char** argv ) {
	try {
		Conf webserver(argc, argv);
	} catch (std::exception& e) {
		std::cerr << R("Error: ") << e.what() << std::endl;
		return (1);
	}
	return (0);
}
