/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nnabaeei <nnabaeei@student.42heilbronn.    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/08 10:39:02 by nnabaeei          #+#    #+#             */
/*   Updated: 2024/09/10 15:47:10 by nnabaeei         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpRequest.hpp"
#include "Server.hpp"


HTTPRequest::HTTPRequest( void ) {}

HTTPRequest::~HTTPRequest( void ) {
}

HTTPRequest::HTTPRequest(std::map<std::string, std::string> const & serverConfig) :
	_requestString(""),
	_method(""),
	_uri(""),
	_version(""),
	_serverConfig(serverConfig) {
}

bool HTTPRequest::isValidMethod(const std::string &method)
{
	return (method == "GET" || method == "POST" || method == "HEAD" || method == "PUT" || method == "DELETE");
}

/*Check The HTTP Version Validity*/
bool HTTPRequest::isValidHttpVersion(const std::string &ver)
{
	return (ver == "HTTP/1.1" || ver == "HTTP/1.0");
}

/*Parse The Received Request And Creat a Map Of Its Headers*/
bool HTTPRequest::parse(ConnectedSocket &connectedSocket)
{
	std::istringstream requestStream(connectedSocket.getRequestHeader());
	std::string line;

	// Parse request line
	if (!std::getline(requestStream, line))
		return (false);
	std::istringstream requestLine(line);
	if (!(requestLine >> _method >> _uri >> _version))
		return (false);

	if (!isValidMethod(_method) || !isValidHttpVersion(_version))
		return (false);

	// Parse headers
	_requestMap["version"] = _version;
	_requestMap["uri"] = _uri;
	_requestMap["method"] = _method;

	while (std::getline(requestStream, line) && line != "\r")
	{
		size_t pos = line.find(":");
		if (pos != std::string::npos)
		{
			std::string key = line.substr(0, pos);
			std::string value = line.substr(pos + 1);
			value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
			value.erase(0, value.find_first_not_of(" "));
			_requestMap[key] = value;
		}
	}

	if (_method == "POST") {
        std::string body;
        while (std::getline(requestStream, line)) {
            body += line + "\n";
        }
        body.erase(body.find_last_not_of("\n") + 1); // Remove the last newline
        _requestMap["Body"] = body;
    }

	//****************print header map********************
	// printRequestMap();
	//****************print server config map**************
	// printServerConfig();
	//****************************************************

	connectedSocket.setRequestMap(this->_requestMap);

	return true;
}

std::string const & HTTPRequest::getRequestString() const {
	return (_requestString);
}

std::map<std::string, std::string> const & HTTPRequest::getRequestMap() {return (_requestMap);}

void HTTPRequest::setServerConfig(std::map<std::string, std::string> const & serverConfig) {
	_serverConfig = serverConfig;
}

void HTTPRequest::displayRequestString() const
{
	std::cout << RED "****Request String:\n"
		<< CYAN << _requestString << RESET << std::endl;
}

std::map<std::string, std::string> &HTTPRequest::getServerConfig(void) {
	return this->_serverConfig;
}

void HTTPRequest::printRequestMap()
{
	std::cout << RED "****Request Map:\n";
	std::map<std::string, std::string>::iterator itr = _requestMap.begin();
	for (; itr != _requestMap.end(); itr++)
		std::cout << ORG << itr->first << ":" MAGENTA << itr->second << RESET << std::endl;
}

void HTTPRequest::printServerConfig()
{
	std::cout << RED "****The server config map:\n";
	std::map<std::string, std::string>::iterator itr = _serverConfig.begin();
	for (; itr != _serverConfig.end(); itr++)
		std::cout << ORG << itr->first << "->" MAGENTA << itr->second << RESET << std::endl;
}

bool HTTPRequest::isHeaderReceived(std::string buffer) {
	size_t bodyStartIndex = buffer.find("\r\n\r\n");
	if (bodyStartIndex == std::string::npos)
		return false;
	else
		return true;
}

std::string getSubStringFromMiddleToIndex(std::string &string, std::string const &toFind, size_t startOffset, size_t endIndex) {
	size_t foundIndex = string.find(toFind);
	if (foundIndex == std::string::npos)
		return "";
	std::string result = string.substr(foundIndex + startOffset, endIndex);
	return result;
}

std::string getSubStringFromStartToIndex(std::string &string, std::string const &toFind) {
	size_t foundIndex = string.find(toFind);
	if (foundIndex == std::string::npos)
		return "";
	std::string result = string.substr(0, foundIndex);
	return result;
}

std::string HTTPRequest::extractContentLength(std::string request) {
	std::string toFind = "Content-Length:";
	std::string contentLength = "";
	std::string temp = getSubStringFromMiddleToIndex(request, toFind, toFind.length(), std::string::npos);
	if (temp.empty())
		return "0";
	contentLength = getSubStringFromStartToIndex(temp, "\r\n");
	return contentLength;
}

std::string HTTPRequest::extractBody(std::string request) {
	std::string toFind = "\r\n\r\n";
	std::string body = getSubStringFromMiddleToIndex(request, toFind, toFind.length(), std::string::npos);
	return body;
}

std::string HTTPRequest::extractHeader(std::string request) {
	std::string result = getSubStringFromStartToIndex(request, "\r\n\r\n");
	if (!result.empty())
		result.append("\r\n\r\n");
	return result;
}

/*It Receives The Client Request, Buffers It And Passes It To The Parse Function.*/
bool HTTPRequest::handleRequest(int connectedSocketFd, pollfd *pollFds, size_t i, ConnectedSocket &connectedSocket)
{
	char buffer[40960];
	ssize_t bytesRead = recv(connectedSocketFd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead == -1)
		return (false);
	if (bytesRead == 0)
		return (false);
	buffer[bytesRead] = '\0';
	_requestString.assign(buffer, bytesRead);

	//****************print request***********************
	// displayRequestString();
	//****************************************************

	std::ostringstream outputStream(std::ios::binary);
	outputStream.write(buffer, bytesRead);
	outputStream.clear();

	if (receiveInChuncks(connectedSocket, pollFds, i, outputStream))
	{
		pollFds[i].events = POLLOUT;
		connectedSocket.setState(DONE);
	}
	else
		return true;
	
	if (!parse(connectedSocket))
		connectedSocket.setState(ERROR);
	return (true);
}

/*It Writes The String To The File Path With Showing Its Scap Characters Inside In The ASCII.*/
void writeStringToFile(std::string request, std::string path)
{
	std::istringstream streamTest(request);
	std::string stringTest;
	std::ofstream outFile(path.c_str());
	while (std::getline(streamTest, stringTest))
	{
		std::cout << ORG << stringTest << RESET << std::endl;
		// Manually escape special characters
		if (outFile.is_open())
		{
			for (std::string::size_type i = 0; i < stringTest.size(); ++i)
			{
				char c = stringTest[i];
				switch (c)
				{
				case '\r': outFile << "'\\r'"; break;
				case '\n': outFile << "'\\n'"; break;
				case '\t': outFile << "'\\t'"; break;
				case '\v': outFile << "'\\v'"; break;
				case '\f': outFile << "'\\f'"; break;
				case ' ' : outFile << "' '"  ; break;
				default: outFile << c;
				}
			}
		}
	}
	outFile.close();
	exit(1);
}

void HTTPRequest::storeHeader(ConnectedSocket &connectedSocket) {
	if (connectedSocket.getRequestHeader().empty()) {
			connectedSocket.appendToHeader(this->extractHeader(connectedSocket.getRequest()));
			connectedSocket.setIsHeaderComplete(true);
		}
		else {
			connectedSocket.appendToHeader(this->extractHeader(this->_requestString));
			connectedSocket.setIsHeaderComplete(true);
		}
}

void HTTPRequest::readAllHeader(ConnectedSocket &connectedSocket, pollfd *pollFds, size_t i) {
	connectedSocket.setConnectionStartTime();
	connectedSocket.setState(READING);
	pollFds[i].events = POLLIN;
}

void HTTPRequest::readAllBody(ConnectedSocket &connectedSocket, pollfd *pollFds, size_t i, std::ostringstream const &outputStringStream) {
	connectedSocket.setConnectionStartTime();
	connectedSocket.appendToBody(outputStringStream);
	pollFds[i].events = POLLIN;
	connectedSocket.setState(READING);
}

bool HTTPRequest::receiveInChuncks(ConnectedSocket &connectedSocket, pollfd *pollFds, size_t i, std::ostringstream const &outputStringStream) {

	connectedSocket.appendToRequest(this->_requestString);
	
	if (!isHeaderReceived(connectedSocket.getRequest())) {
		this->readAllHeader(connectedSocket, pollFds, i);
	}

	if (isHeaderReceived(connectedSocket.getRequest()))
	{
		this->storeHeader(connectedSocket);
			
		if (!connectedSocket.getContentLength())
		{
			std::string contentLength = this->extractContentLength(connectedSocket.getRequest());
			connectedSocket.setRequestBodyLength(contentLength);
		}
		if (connectedSocket.getContentLength() && connectedSocket.getRequestBody().str().empty()) {
			std::string toAppend = this->extractBody(connectedSocket.getRequest());
			std::ostringstream outputString;
			outputString.clear();
			outputString.write(toAppend.c_str(), toAppend.length());
			outputString.flush();
			connectedSocket.appendToBody(outputString);
			pollFds[i].events = POLLIN;
			connectedSocket.setAvoidBodyFirstChunckRepeat(true);
		}

		if (!connectedSocket.getRequestBody().str().empty() && connectedSocket.getRequestBody().str().size() < connectedSocket.getContentLength() && !connectedSocket.getAvoidBodyFirstChunckRepeat())
		{
			this->readAllBody(connectedSocket, pollFds, i, outputStringStream);
			std::cout << MAGENTA << "Body received size: " << connectedSocket.getRequestBody().str().size() << RESET << std::endl;
			std::cout << MAGENTA << "Content-Length: " << connectedSocket.getContentLength() << RESET << std::endl;
		}

		connectedSocket.setAvoidBodyFirstChunckRepeat(false);

	}
	if (connectedSocket.getRequestBody().str().size() < connectedSocket.getContentLength())
		return false;
	else
		return true;
}