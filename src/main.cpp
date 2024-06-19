/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fahmadia <fahmadia@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 10:24:40 by ncasteln          #+#    #+#             */
/*   Updated: 2024/06/19 14:52:44 by fahmadia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.h"
#include "colors.h"
#include "Http.hpp"
#include "ListeningSocket.hpp"
#include "ConnectedSocket.hpp"
#include "HttpServer.hpp"

int main ( int argc, char** argv ) {
	// std::cout << B("*************** WEBSERV! ***************") << std::endl;
	// try {
	// 	Http webserver(argc, argv);
	// } catch (std::exception& e) {
	// 	std::cerr << "Error: " << e.what() << std::endl;
	// 	return (1);
	// }

	// if (!createServer())
	// 	return 1;
	(void)argc;
	(void)argv;

	try {
		HttpServer httpServer;
		ListeningSocket const &listeningSocket = httpServer.getListeningSocket();
		listeningSocket.setPortAvailable();
		listeningSocket.bindSocket();
		listeningSocket.listenToRequests();
		listeningSocket.acceptFirstRequestInQueue();
	} catch(Exception const &exception) {
		std::cerr << RED << exception.what() << RESET << std::endl;
		return exception.getError();
	}
	return (0);
}
