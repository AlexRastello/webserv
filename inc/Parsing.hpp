#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

struct serverLocationSt
{
	bool						uploadable;
	bool						autoindex;
	std::string 				route;
	std::vector<std::string>	methods;
};

struct serverSt
{
	std::vector<int>				port;
	std::string						adress;
	std::string 					index;
	std::string						cgiDir;
	std::string 					root;
	std::string						errorPage;
	std::string						redirectPage;
	std::string						uploadDir;
	std::string						uploadPage;
	std::string						cookiePageAdd;
	std::string						cookiePageDelete;
	int								bodySize;
	std::vector<std::string>		serverName;
	std::vector<std::string>		cgiExt;
	std::vector<serverLocationSt>	serverLocations;
};

class Parsing
{
	public:
		Parsing();
		virtual~Parsing();
		bool					setupParsing(std::string const & path);
		void					printParsing();
		std::vector<serverSt> &	getServers();
	private:
		bool	_extractServerLocation(std::vector<serverLocationSt> & vctLocation, std::vector<std::string> const & content);
		bool	_extractStr(std::string & str, std::string const & line);
		bool	_extractBool(bool & val, std::string const & line);
		bool	_extractBodySize(int & bodySize, std::string const & line);
		bool	_extractPort(std::vector<int> & port, std::string const & line);
		bool	_extractVctStr(std::vector<std::string> & cgiExt, std::string const & line);
		bool	_missingArgForServer(serverSt const & server);
		bool	_parseServer(std::vector<std::string> const & content);
		bool	_bracketsClose(std::vector<std::string> const & content);
		bool	_errorInFile(std::string const & msg);

		void	_eraseSpace(std::string & str);
		void	_initPars(std::vector<std::vector<std::string> > & contentServers, std::vector<std::string> const & content);
		std::vector<serverSt>	_servers;
};