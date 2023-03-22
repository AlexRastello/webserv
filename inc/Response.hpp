#pragma once

#include <err.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/types.h>
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
#include "Request.hpp"
#include "MimeType.hpp"

class Request;

class Response
{
	public:
		Response();
		virtual~Response();
		void		sendResponse(Request request, int sock);

	private:
		void    	_sendToHttp(Request request, int sock);
		void		_formatHtml(Request request, std::string &html);
		std::string _getContent(std::string path);
		std::string	_getInputCgi(Request request);
};