/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Poll.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnabaeei <nnabaeei@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/19 10:55:19 by ncasteln          #+#    #+#             */
/*   Updated: 2024/09/10 15:07:01 by nnabaeei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <csignal>
#include "Poll.hpp"

bool stopServer = false;
int Poll::cgiChildProcessNum = 0;

void printCurrentPollFdsTEST(nfds_t currentMonitored, struct pollfd* pollFd) {
	nfds_t i = 0;
	std::cout << BLUE << "CURRENT PollFds List: " << RESET << std::endl;
	while (i < currentMonitored) {
		std::cout << "totalFds[" << i << "].fd = " << pollFd[i].fd << std::endl;
		std::cout << "totalFds[" << i << "].events = " << std::hex << pollFd[i].events << std::dec << std::endl;
		std::cout << "totalFds[" << i << "].revents = " << std::hex << pollFd[i].revents << std::dec << std::endl;

		if (i == 4 || i == 0){
			Server::logMessage("totalFds[" + Server::intToString(i) + "].revents = " + Server::intToString(pollFd[i].revents));
		}
		i++;
	}
	return;
}

Poll::Poll(const Parser &configuration) : _serverList(std::vector<Server>()),
	_currentMonitored(0),
	_totalMonitored(0),
	_totalFds(NULL)
{
	Server::logMessage("INFO: The Poll Constructor Called!");
	createServers(configuration);
	initFds();
}
Poll::~Poll(void)
{
	delete[] _totalFds;
	this->_totalFds = NULL;
	
};

void Poll::createServers(const Parser &configuration)
{
	Server::logMessage("INFO: The Servers In The Poll Constructor Created!");
	std::vector<ServerConf>::const_iterator serverConfIt = configuration.getHttp().getServer().begin();
	while (serverConfIt != configuration.getHttp().getServer().end())
	{
		if (_serverList.size() > 0)
		{
			std::map<std::string, std::string> currentServerSettings = (*serverConfIt).getSettings();
			if (mergeServerWithSamePort(currentServerSettings))
			{
				serverConfIt++;
				continue;
			}
		}
		Server s((*serverConfIt).getSettings(), (*serverConfIt).getLocation());
		_serverList.push_back(s);
		serverConfIt++;
	}
}

bool Poll::mergeServerWithSamePort(std::map<std::string, std::string> serverConf)
{
	std::vector<Server>::iterator it = _serverList.begin();
	while (it != _serverList.end())
	{
		if ((*it).getPort() == serverConf["port"])
		{
			(*it).addServerName(serverConf["server_name"]);
			(*it).addRoot(serverConf["root"]);
			(*it).addIndex(serverConf["index"]);
			return (true);
		}
		it++;
	}
	return (false);
}

void Poll::init(void)
{
	Server::logMessage("INFO: The Poll On The Server List start To set!");
	std::vector<Server>::iterator serverIt = _serverList.begin();

	while (serverIt != _serverList.end())
	{
		(*serverIt).setPortAvailable();
		(*serverIt).bindSocket();
		(*serverIt).listenToRequests();
		serverIt++;
	}
}

void signalHandler(int sigNum) {
	if (sigNum == SIGINT)
		stopServer = true;
}

void Poll::start(void)
{
	while (true)
	{
		signal(SIGPIPE, SIG_IGN);
		signal(SIGINT, signalHandler);
		try
		{
			int eventsNum = poll(_totalFds, _currentMonitored, 10000);
			cleanConnectedSockets();
			if (eventsNum < 0)
			{
				Exception pollException("Poll Interruption by a signal", POLL_FAILED);
				throw pollException;
			}
			if (eventsNum > 0)
			{
				handleEvent();
			}

			if (stopServer)
				break;
		}
		catch (Exception const &exception)
		{
			throw exception;
		}
	}
}

void Poll::handleEvent(void)
{
	std::vector<Server>::iterator serverIt = _serverList.begin();

	size_t i = 0;
	while (serverIt != _serverList.end())
	{
		if (_totalFds[i].fd == (*serverIt).getListeningSocket().getSocketFd())
		{
			try {
				handleListeningEvent(i, (*serverIt));
				serverIt++;
				i++;
			} catch(Exception const &exception) {
				throw exception;
			}
		}
		else {
			serverIt++;
			i++;
		}
	}
	serverIt = this->_serverList.begin();
	while (serverIt != _serverList.end())
	{
		i = 0;
		std::map<int, ConnectedSocket>::iterator connectedSocketIt;
		std::map<int, ConnectedSocket>::iterator connectedSocketItEnd = serverIt->getConnectedSockets().end();

		for (connectedSocketIt = serverIt->getConnectedSockets().begin(); connectedSocketIt != connectedSocketItEnd; connectedSocketIt++)
		{
			handleConnectedEvent(connectedSocketIt->second.getSocketFd(), (*serverIt), &connectedSocketIt);
			if (connectedSocketIt->second.getIsCgiChildProcessReturning() == true && connectedSocketIt->second.getIsCgi() == true)
			{
				std::cerr << RED << "STOP " << connectedSocketIt->second.getIsCgiChildProcessReturning() << RESET << std::endl;
				stopServer = true;
			}
			i++;
			if (connectedSocketIt == connectedSocketItEnd)
				break;
		}
		serverIt++;
	}
}

void Poll::handleListeningEvent(size_t i, Server &s)
{
		if ((this->_totalFds[i].revents & POLLERR) || (this->_totalFds[i].revents & POLLHUP) || (this->_totalFds[i].revents & POLLNVAL))
		{
			Server::logMessage("ERROR: Listening Socket Failed.");
			Exception pollException("Listening Socket Failed.", LISTENING_FAILED);
			throw pollException;
		}
		if ((_totalFds[i].revents & POLLIN))
		{
			Server::logMessage(" * event happened on listeningSocket: " + Server::intToString( _totalFds[i].fd));
			if (this->isMaxConnection(s, i))
				return;

			int connectedSocketFd = s.acceptFirstRequestInQueue(true);
			Server::logMessage("connected socket [" + Server::intToString(connectedSocketFd) +"] is created" );
			if (connectedSocketFd < 0)
				return;
			addConnectedSocketToMonitoredList(connectedSocketFd);
			this->_totalFds[i].revents = 0;
		}
}

void Poll::handleConnectedEvent(int connectedSocketFd, Server &s, std::map<int, ConnectedSocket>::iterator *connectedSocketIt)
{
		nfds_t i = this->mapConnectedSocketFdToPollFd(connectedSocketFd);
		if ((_totalFds[i].revents & POLLIN)) {
			if (!this->receiveRequest(s, i, connectedSocketFd, connectedSocketIt))
				return;
		}
		if ((this->_totalFds[i].revents & POLLERR) || (this->_totalFds[i].revents & POLLHUP))
		{
			int closeResult = 0;
			if (this->_totalFds[i].fd >= 0)
				closeResult = close(this->_totalFds[i].fd);
		
			if (closeResult == -1)
				Server::logMessage("ERROR: POLL ERROR - CLOSING FAILED! fd: " + Server::intToString(this->_totalFds[i].fd));
		}
		if (((this->_totalFds[i].revents & POLLERR) || (this->_totalFds[i].revents & POLLHUP) || (this->_totalFds[i].revents & POLLNVAL)) && (this->_totalFds[i].fd != -1)) {
			s.getConnectedSockets()[connectedSocketFd].setIsConnected(false);
			std::map<int, ConnectedSocket>::iterator temp = *connectedSocketIt;
			temp++;
			s.getConnectedSockets().erase(*connectedSocketIt);
			*connectedSocketIt = temp;
			this->_totalFds[i].fd = -1;
			this->_totalFds[i].revents = 0;
			this->removeClosedSocketsFromPollFds();
			return;
		}
		else if (_totalFds[i].revents & POLLOUT)
			this->sendResponse(s, i, connectedSocketFd, connectedSocketIt);
}

void Poll::addConnectedSocketToMonitoredList(int connectedSocketFd)
{
	for (size_t i = 0; i < _totalMonitored; i++)
	{
		if (_totalFds[i].fd == -1)
		{
			_totalFds[i].fd = connectedSocketFd;
			_totalFds[i].events = POLLIN;
			break;
		}
	}
	if (this->_currentMonitored >= this->_totalMonitored)
		return;
	_currentMonitored++;
	if (_currentMonitored > _totalMonitored)
		Server::logMessage("ERROR: CurrentMonitored Fd Is Bigger Than The _totalMonitored!");
}

void Poll::initFds(void)
{
	size_t n_servers = _serverList.size();
	_totalMonitored = (n_servers * MAX_CONNECTIONS) + n_servers;
	_currentMonitored = static_cast<nfds_t>(n_servers);

	_totalFds = new struct pollfd[_totalMonitored];
	memset(this->_totalFds, 0, sizeof(struct pollfd) * this->_totalMonitored);
	std::vector<Server>::iterator it = _serverList.begin();
	size_t i = 0;

	while (i < this->_totalMonitored)
	{
		this->_totalFds[i].fd = -1;
		i++;
	}

	i = 0;
	while (it != _serverList.end())
	{
		_totalFds[i].fd = (*it).getListeningSocket().getSocketFd();
		_totalFds[i].events = POLLIN;

		if (fcntl(_totalFds[i].fd, F_SETFL, O_NONBLOCK) == -1)
			Server::logMessage("ERROR: The fcntl F_SETFL Error Set!");
		it++;
		i++;
	}
}

void Poll::removeClosedSocketsFromMap(Server &s) {
	if (!(s.getConnectedSockets().size()))
		return;
	std::map<int, ConnectedSocket>::iterator it = s.getConnectedSockets().begin();
	while (it != s.getConnectedSockets().end()) {
		if (it->second.getIsConnected() == false) {
			std::map<int, ConnectedSocket>::iterator temp = it;
			temp++;
			s.getConnectedSockets().erase(it);
			it = temp;
		}
		else {
			it++;
		}
	}
}

void Poll::removeClosedSocketsFromPollFds(void) {
	for (nfds_t i = 0; i < this->_currentMonitored;)
		{
			if (this->_totalFds[i].fd == -1)
			{
				this->_totalFds[i].events = 0;
				this->_totalFds[i].revents = 0;
				for (nfds_t j = i; j < this->_currentMonitored - 1; ++j)
				{
					this->_totalFds[j].fd = this->_totalFds[j + 1].fd;
					this->_totalFds[j].events = this->_totalFds[j + 1].events;
					this->_totalFds[j].revents = this->_totalFds[j + 1].revents;
					this->_totalFds[j + 1].fd = -1;
					this->_totalFds[j + 1].events = 0;
					this->_totalFds[j + 1].revents = 0;
				}
				this->_currentMonitored--;
			}
			else
			{
				i++;
			}
		}
	return;
}

void Poll::printCurrentPollFds(void) {
	nfds_t i = 0;
	std::cout << BLUE << "CURRENT PollFds List: " << RESET << std::endl;
	while (i < this->_currentMonitored) {
		std::cout << "totalFds[" << i << "].fd = " << this->_totalFds[i].fd << std::endl;
		std::cout << "totalFds[" << i << "].events = " << std::hex << this->_totalFds[i].events << std::dec << std::endl;
		std::cout << "totalFds[" << i << "].revents = " << std::hex << this->_totalFds[i].revents << std::dec << std::endl;
		i++;
	}
	return;
}

void Poll::printAllPollFds(void) {
	nfds_t i = 0;
	std::cout << BLUE << "ALL PollFds List: " << RESET << std::endl;
	while (i < this->_totalMonitored) {
		std::cout << "totalFds[" << i << "].fd = " << this->_totalFds[i].fd << std::endl;
		std::cout << "totalFds[" << i << "].events = " << this->_totalFds[i].events << std::endl;
		std::cout << "totalFds[" << i << "].revents = " << this->_totalFds[i].revents << std::endl;
		i++;
	}
	return;
}

nfds_t Poll::mapConnectedSocketFdToPollFd(int connectedSocketFd) {
	for (nfds_t i = 0; i < this->_currentMonitored; i++)
	{
		if (this->_totalFds[i].fd == connectedSocketFd)
			return i;
	}
	return 0;
}

void Poll::closeTimedoutSockets(nfds_t pollNum, ConnectedSocket &connectedSocket) {
	if (this->_totalFds[pollNum].fd > 0) {
			close(this->_totalFds[pollNum].fd);
		}
		connectedSocket.setIsConnected(false);
		this->_totalFds[pollNum].fd = -1;
}

void Poll::cleanConnectedSockets() {
	std::vector<Server>::iterator serverIt;
	std::vector<Server>::iterator serverItEnd = this->_serverList.end();

	for (serverIt = this->_serverList.begin(); serverIt != serverItEnd; serverIt++)
	{
			std::map<int, ConnectedSocket>::iterator connectedSocketIt = serverIt->getConnectedSockets().begin();
			std::map<int, ConnectedSocket>::iterator connectedSocketItEnd = serverIt->getConnectedSockets().end();

			while (connectedSocketIt != connectedSocketItEnd)
			{
				nfds_t pollNum = this->mapConnectedSocketFdToPollFd(connectedSocketIt->second.getSocketFd());
				if (pollNum < this->_serverList.size() )
				{
					connectedSocketIt->second.setIsConnected(false);
					std::map<int, ConnectedSocket>::iterator temp = connectedSocketIt;
					temp++;
					serverIt->getConnectedSockets().erase(connectedSocketIt);
					connectedSocketIt = temp;
					continue;
				}

				time_t now = time(NULL);
				if (serverIt->getKeepAliveTimeout() && (serverIt->getKeepAliveTimeout() + connectedSocketIt->second.getConnectionStartTime() < now ) && !(this->_totalFds[pollNum].revents & POLLIN) && !(this->_totalFds[pollNum].revents & POLLOUT)) {
					closeTimedoutSockets(pollNum, connectedSocketIt->second);
				}
				else if (!(serverIt->getKeepAliveTimeout()) && !(this->_totalFds[pollNum].revents & POLLIN) && !(this->_totalFds[pollNum].revents & POLLOUT)) {
					closeTimedoutSockets(pollNum, connectedSocketIt->second);
				}

				connectedSocketIt++;
			}
			this->removeClosedSocketsFromMap(*serverIt);
			this->removeClosedSocketsFromPollFds();
	}
}

bool Poll::isMaxConnection(Server &s, size_t i) {
	if (s.getConnectedSockets().size() >= MAX_CONNECTIONS) {
		int connectedSocketFd = s.acceptFirstRequestInQueue(false);
		this->_totalFds[i].revents = 0;
		std::cout << "Server " << this->_totalFds[i].fd << " has reached the max " << MAX_CONNECTIONS << " capacity!" << std::endl;
		if (connectedSocketFd >= 0)
			close(connectedSocketFd);
		return true;
	}
	return false;
}

bool Poll::receiveRequest(Server &s, size_t i, int connectedSocketFd, std::map<int, ConnectedSocket>::iterator *connectedSocketIt) {
	(void)connectedSocketFd;
	if (s.getHttpReq().handleRequest(this->_totalFds[i].fd, this->_totalFds, i, s.getConnectedSockets()[connectedSocketFd]))
	{
		if (!(this->_totalFds[i].revents & POLLHUP) && ((*connectedSocketIt)->second.getState() == DONE || (*connectedSocketIt)->second.getState() == ERROR)) {
			s.getHttpResp().handleResponse(this->_totalFds[i].fd, POLLIN_TMP, this->_totalFds, i, s.getConnectedSockets()[connectedSocketFd]);
		}
		return (true);
	} else {
		int closeResult = 0;
		if (this->_totalFds[i].fd >= 0)
			closeResult = close(this->_totalFds[i].fd);
		if (closeResult == -1)
			Server::logMessage("ERROR: Socket Close Failed.");
		s.getConnectedSockets()[connectedSocketFd].setIsConnected(false);
		std::map<int, ConnectedSocket>::iterator temp = *connectedSocketIt;
		temp++;
		s.getConnectedSockets().erase(*connectedSocketIt);
		*connectedSocketIt = temp;
		this->_totalFds[i].fd = -1;
		this->_totalFds[i].revents = 0;
		this->removeClosedSocketsFromPollFds();
	}
	return (false);
}

void Poll::sendResponse(Server &s, size_t i, int connectedSocketFd, std::map<int, ConnectedSocket>::iterator *connectedSocketIt) {

	if ((*connectedSocketIt)->second.getIsCgi() ) {
		std::string cgiResponse = waitForCgiResponse((*connectedSocketIt)->second, s);
		if (cgiResponse.empty())
			return;
	}

	if(!(s.getHttpResp().handleResponse(this->_totalFds[i].fd, POLLOUT_TMP, this->_totalFds, i, s.getConnectedSockets()[connectedSocketFd]))) {
		int closeResult = 0;
		if (this->_totalFds[i].fd >= 0)
			closeResult = close(this->_totalFds[i].fd);
		if (closeResult == -1)
			Server::logMessage("ERROR: Socket Close Failed.");
		s.getConnectedSockets()[connectedSocketFd].setIsConnected(false);
		std::map<int, ConnectedSocket>::iterator temp = *connectedSocketIt;
		temp++;
		s.getConnectedSockets().erase(*connectedSocketIt);
		*connectedSocketIt = temp;
		this->_totalFds[i].fd = -1;
		this->_totalFds[i].revents = 0;
		this->removeClosedSocketsFromPollFds();
		return;
	}
		
	s.getConnectedSockets()[connectedSocketFd].clearRequestProperties();
	time_t now = time(NULL);
	if ( ((*connectedSocketIt)->second.getState() == WRITING) || (s.getKeepAliveTimeout() && (now < s.getConnectedSockets()[connectedSocketFd].getConnectionStartTime() + s.getKeepAliveTimeout()))) {
		return;
	}


	int closeResult = 0;
	if (this->_totalFds[i].fd >= 0)
		closeResult = close(this->_totalFds[i].fd);
	if (closeResult == -1)
		Server::logMessage("ERROR: Socket Close Failed.");
	s.getConnectedSockets()[connectedSocketFd].setIsConnected(false);
	std::map<int, ConnectedSocket>::iterator temp = *connectedSocketIt;
	temp++;
	s.getConnectedSockets().erase(*connectedSocketIt);
	*connectedSocketIt = temp;
	this->_totalFds[i].fd = -1;
	this->removeClosedSocketsFromPollFds();
}

std::string Poll::waitForCgiResponse(ConnectedSocket &connectedSocket, Server &s) {
	int status = 0;
	int result = 0;

	if (connectedSocket.getState() != WRITING && connectedSocket.getState() != READING)
		result = waitpid(connectedSocket.getChildProcessData().id, &status, WNOHANG);

	if (std::time(NULL) > connectedSocket.getCgiStartTime() + CGI_TIMEOUT) {
		HTTPResponse temp;
		std::string response = temp.generateErrorPage(500);
		finishCgi(connectedSocket, s, response);
		kill(connectedSocket.getChildProcessData().id, SIGKILL);
		return response;
	}

	if (result == connectedSocket.getChildProcessData().id || connectedSocket.getState() == READING) {
		if (WIFEXITED(status)) {

			if (WEXITSTATUS(status) == 0)
			{
				std::string response = this->cgiChildProcessSuccess(connectedSocket, s);
				return response;
			}
			else {
				std::string response = this->cgiChildProcessFail(connectedSocket, s);
				return response;
			}
		}
		else {
				std::string response = this->cgiChildProcessFail(connectedSocket, s);
				return response;
		}
	}	else if (result == -1) {
			HTTPResponse temp;
			std::string response = temp.generateErrorPage(500);
			 
			this->finishCgi(connectedSocket, s, response);
			return response;
	}
	else
		return "";

	return "";
}

void Poll::finishCgi(ConnectedSocket &connectedSocket, Server &s, std::string const &response) {
	if (connectedSocket.getChildProcessData().pipeFds[0] != -1)
	{
		Server::logMessage("Parent Process closed " + Server::intToString(connectedSocket.getChildProcessData().pipeFds[0]));
		close(connectedSocket.getChildProcessData().pipeFds[0]);
	}
	connectedSocket.getChildProcessData().pipeFds[0] = -1;
	connectedSocket.setState(DONE);
	s.getHttpResp().setResponseForAConnectedSocket(response, connectedSocket.getSocketFd());
	connectedSocket.setIsCgi(false);
	connectedSocket.setIsCgiChildProcessSuccessful(false);
	connectedSocket.setCgiStartTime();
	connectedSocket.setCgiBuffer("");
	connectedSocket.setIsCgiChildProcessReturning(false);
	connectedSocket.setCgiScriptExtension("");

	Poll::cgiChildProcessNum--;
	return;
}

void makeCGIResultFormed(std::string *cgiResult) {
	std::string html = *cgiResult;

	*cgiResult = "<!DOCTYPE html>\r\n<html lang=\"en\">\r\n<head>\r\n"
	"<meta charset=\"UTF-8\">\r\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\r\n"
	"<title>CGI Execution Result</title>\r\n<style>\r\nbody {font-family: Arial, sans-serif; margin: 20px;}\r\n"
	"pre { background-color: #f4f4f4; padding: 10px; border: 1px solid #ddd; overflow-x: auto;}\r\n</style>\r\n"
	"</head>\r\n<body>\r\n<h1>CGI Execution Result</h1>\r\n<pre><code>\r\n"
	+ html + "\r\n</code></pre>\r\n</body>\r\n</html>";
}

std::string Poll::cgiChildProcessSuccess(ConnectedSocket &connectedSocket, Server &s) {
	connectedSocket.setIsCgiChildProcessSuccessful(true);

	char buffer[1024];
	buffer[1023] = '\0';

	if (fcntl(connectedSocket.getChildProcessData().pipeFds[0], F_SETFL, O_NONBLOCK) == -1)
		Server::logMessage("ERROR: The fcntl F_SETFL Error Set!");

	int result = read(connectedSocket.getChildProcessData().pipeFds[0], buffer, sizeof(buffer) - 1);

	if (result > 0) {
		connectedSocket.setState(READING);
		std::string temp(buffer, result);
		connectedSocket.appendToCgiBuffer(temp);
		return "";
	}
	std::string content = connectedSocket.getCgiBuffer();
	makeCGIResultFormed(&content);

	connectedSocket.setCgiBuffer("");

	std::ostringstream ostring;
	ostring << "HTTP/1.1 200 OK\r\n";
	ostring << "Content-Type: text/html\r\n";
	ostring << "Connection: close\r\n";
	ostring << "Content-Length: " << content.length() << "\r\n\r\n";
	ostring << content;

	std::string response = ostring.str();
	finishCgi(connectedSocket, s, response);
	return response;
}

std::string Poll::cgiChildProcessFail(ConnectedSocket &connectedSocket, Server &s) {
	std::string response = s.getHttpResp().generateErrorPage(500);
	finishCgi(connectedSocket, s, response);
	return response;
}
