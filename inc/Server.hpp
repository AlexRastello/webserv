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
#include <fcntl.h>
#include "Request.hpp"
#include "Response.hpp"
#include "Parsing.hpp"


#define NUM_CLIENTS 100
#define MAX_MSG_SIZE 10000
#define MAX_EVENTS 32

class clientSock {
	public:
		clientSock() {};
		~clientSock() {};
		void	setFd(int const fd) { this->_fd = fd;}
		int		getFd() { return (this->_fd); }
	private:
    	int _fd;
};

class Server
{
	public:
		Server(serverSt parsingServer, int port);
		virtual~Server();
		void    					initServ();
	private:
		int		_getConn(int fd);
		int		_connAdd(int fd);
		int		_connDel(int fd);
		void	_handleRecv(int sock, Request &request, std::vector<struct kevent> &vct, bool &isRecv);
		void 	_runEventLoop();
		bool 	_mainSocket();
		clientSock			_clients[SOMAXCONN];
		unsigned long		_listener;
		serverSt			_parsingServer;
		int					_port;
};