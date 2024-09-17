/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fahmadia <fahmadia@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/18 13:10:43 by fahmadia          #+#    #+#             */
/*   Updated: 2024/09/10 10:09:47 by fahmadia         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <iostream>
#include <ctime>
#include <sys/stat.h>
#include <string>
#include <netdb.h>
#include <fcntl.h>
#include <stdexcept>
#include <vector>
#include <arpa/inet.h>
#include <poll.h>
#include <map>
#include <fstream>
#include <time.h>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <sstream>
#include "Exception.hpp"
#include "../sockets/listening-socket/ListeningSocket.hpp"
#include "../sockets/connected-socket/ConnectedSocket.hpp"
#include "../server/Server.hpp"
#include "HttpRequest.hpp"

#define MAX_CONNECTIONS 130
#define CGI_TIMEOUT 100
#define MAX_CGI_CHILD_PROCESSES 5

class Server
{
private:
	const std::string _port;
	std::vector<std::string> _serverNames;
	std::vector<std::string> _roots;
	std::vector<std::string> _indexes;
	int _keepAliveTimeout;
	ListeningSocket _listeningSocket;
	std::map<int, ConnectedSocket> _connectedSockets;
	HTTPRequest _httpReq; 
	HTTPResponse _httpResp; 

	Server(void);
	Server &operator=(Server const &rhs);
	
	std::string readHtmlFile(std::string path);

public:
	Server(std::map<std::string, std::string> settings, std::vector<LocationConf> const &locations);
	~Server(void);
	Server(const Server &other);

	ListeningSocket const &getListeningSocket(void) const;
	const std::string getPort(void) const;
	std::map<int, ConnectedSocket> &getConnectedSockets(void);
	HTTPRequest & getHttpReq(); 
	HTTPResponse & getHttpResp(); 
	int getKeepAliveTimeout(void) const;

	void setKeepAliveTimeout(int keepAliveTimeout);

	void addServerName(std::string newName);
	void addRoot(std::string newRoot);
	void addIndex(std::string newIndex);
	void setPortAvailable(void);
	void bindSocket(void) const;
	void listenToRequests(void) const;
	int acceptFirstRequestInQueue(bool addToConnectedSocketsList);
	void printConnectedSockets(void);
	static int stringToInt(const std::string &str);
	static std::string intToString(int const i);
	static void logMessage(const std::string &message); 

};

#endif
