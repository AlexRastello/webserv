#pragma once
#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <vector>
#include <ctime>
#include "Parsing.hpp"
#include "MimeType.hpp"

class Request
{
	public:
		Request(serverSt const & serverParsing, int const & port);
		virtual~Request();
		
		std::vector<std::string>    extractRouteFromRequest(std::string const & requestFirstLine);
		std::string					formatRoute(std::string const & s1, std::string const & s2);
		std::string					formatRoute(std::string const & s);
		void						formatBody();
		void						insertRequestInBody(std::string const & request);
		void						parseRequest(std::string const & request);

		void						setRoute(std::string const &route);
		void						setCodeRequest(int const & codeRequest);
        void						setStatusRequest(std::string const & statusRequest);
        void						setContentType(std::string const & contentType);

		int const &				getPort() const;
		int const &				getCodeRequest() const;
		bool const &	        getIsAutoIndex() const;
		size_t		 			getContentLength() const;
		std::string const &	    getStatusRequest() const;
		std::string const &		getRoute() const;
		std::string const &		getRouteNotFound() const;
		std::string const &		getFilenameUpload() const;
		std::string const &		getMethod() const;
		std::string const &		getBody() const;
		std::string const &		getContentType() const;
		std::string const &		getInputCgi() const;
		std::string const &		getErrorRequest() const;
		std::string const &		getServerName() const;
		serverSt const &		getServerParsing() const;
		void					clear();

	private:
		void                        		_successInRequest(int const & codeRequest, std::string const & statusRequest, std::string const & route, std::string const & contentType);
		void                       			_errorInRequest(int const & codeRequest, std::string const & statusRequest, std::string const & routeNotFound);
		void                        		_extractBodySizeAndWebkit(std::string const & request);
		std::vector<std::string>    		_extractRouteFromLocation(std::string const & locationRoute);
		std::string 						_extractFilenameUpload(std::vector<std::string> const & vct);
		std::vector<std::string>			_getVctRequest(std::string const & request);
		std::vector<std::string>			_getVctRequestMethod(std::string const & request);

		bool						_isEraseFirstBound;
		bool						_isAutoIndex;
		int							_codeRequest;
		int							_contentLength;
		std::string					_request;
		std::string					_inputCgi;
		std::string					_route;
		std::string					_routeNotFound;
		std::string					_filenameUpload;
		std::string					_method;
		std::string					_body;
		std::string					_contentType;
		std::string					_statusRequest;
		std::string					_webkit;
		std::string					_serverName;
		serverSt					_serverParsing;
		int							_port;
};