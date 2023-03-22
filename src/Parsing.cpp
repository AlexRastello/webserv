#include "Parsing.hpp"

Parsing::Parsing(){}

Parsing::~Parsing(){}

std::vector<serverSt> &	Parsing::getServers() { return (this->_servers); }

void	Parsing::printParsing() {
	for (std::vector<serverSt>::iterator itServ = this->_servers.begin(); itServ != this->_servers.end(); itServ++) {
		std::cout << "---Server begin---\n";
		for (std::vector<int>::iterator itPort = itServ->port.begin(); itPort != itServ->port.end(); itPort++)
			std::cout << "Port => " << *itPort << std::endl;
		std::cout << "Adress => " << itServ->adress << std::endl;
		std::cout << "Index => " << itServ->index << std::endl;
		std::cout << "Cgi_dir => " << itServ->cgiDir << std::endl;
		std::cout << "Root => " << itServ->root << std::endl;
		std::cout << "Error_page => " << itServ->errorPage << std::endl;
		std::cout << "Body_size => " << itServ->bodySize << std::endl;
		std::cout << "Redirect_page => " << itServ->redirectPage << std::endl;
		std::cout << "Upload_dir => " << itServ->uploadDir << std::endl;
		std::cout << "Upload_page => " << itServ->uploadPage << std::endl;
		std::cout << "Cookie_page_add => " << itServ->cookiePageAdd << std::endl;
		std::cout << "Cookie_page_delete => " << itServ->cookiePageDelete << std::endl;
		for (std::vector<std::string>::iterator itServName = itServ->serverName.begin(); itServName != itServ->serverName.end(); itServName++)
		std::cout << "Server_name => " << *itServName << std::endl;
		for (std::vector<std::string>::iterator itCgi = itServ->cgiExt.begin(); itCgi != itServ->cgiExt.end(); itCgi++)
			std::cout << "Cgi ext => " << *itCgi << std::endl;
		for (std::vector<serverLocationSt>::iterator itLocation = itServ->serverLocations.begin(); itLocation != itServ->serverLocations.end(); itLocation++) {
			std::cout << "---server_location---\n";
			std::cout << std::boolalpha;
			std::cout << "Route => " << itLocation->route << std::endl;
			for (std::vector<std::string>::iterator itMethod = itLocation->methods.begin(); itMethod != itLocation->methods.end(); itMethod++)
				std::cout << "Method => " << *itMethod << std::endl;
			std::cout << "Uploadable => " << itLocation->uploadable << std::endl;
			std::cout << "Autoindex => " << itLocation->autoindex << std::endl;
		}
		std::cout << "---Server end---\n\n";
	}
}

bool	Parsing::setupParsing(std::string const & path) {
	std::ifstream file(path);
	if (!file.is_open())
		return (this->_errorInFile("Error: no such file or directory"));
	
	//Get content
	std::vector<std::string> content;
	std::string line;
	while (std::getline(file, line))
		content.push_back(line);

	//Check bracket
	if (!this->_bracketsClose(content))
		return (this->_errorInFile("Error: brackets don't close"));
	
	//Parse information
	std::vector<std::vector<std::string> > contentServers;
	this->_initPars(contentServers, content);
	for (std::vector<std::vector<std::string> >::const_iterator it = contentServers.begin(); it != contentServers.end(); it++) {
		if (!this->_parseServer(*it))
			return (false);
	}
	return (true);
}

bool	Parsing::_extractServerLocation(std::vector<serverLocationSt> & vctLocation, std::vector<std::string> const & content) {
	(void) vctLocation;
	for (std::vector<std::string>::const_iterator it = content.begin(); it != content.end(); it++)
		std::cout << "Server location " << *it << std::endl;
	return (true);
}

bool	Parsing::_extractPort(std::vector<int> & port, std::string const & line) {
	std::vector<std::string>	vctTmp;
	std::string tmp = line;
	size_t	toFind = tmp.find("=");
	if (toFind != std::string::npos) {
		tmp.erase(0, toFind + 1);
	}
	else
		return (false);
	toFind = tmp.find(",");
	while (toFind != std::string::npos) {
		std::string toPush(tmp, 0, toFind);
		this->_eraseSpace(toPush);
		vctTmp.push_back(toPush);
		tmp.erase(0, toFind + 1);
		toFind = tmp.find(",");
	}
	if (tmp.size() > 0) {
		this->_eraseSpace(tmp);
		if (tmp.size() > 0)
			vctTmp.push_back(tmp);
	}
	for (std::vector<std::string>::iterator it = vctTmp.begin(); it != vctTmp.end(); it++) {
		int	tmpPort;
		try
		{
			tmpPort = std::stoi(*it);
		}
		catch(const std::exception& e)
		{
			return (this->_errorInFile("Error: not a number for port"));
		}
		if (it->size() > 1 && *it != "0" && tmpPort == 0)
			return (this->_errorInFile("Error: not a number for port"));
		if (std::to_string(tmpPort) != *it)
			return (this->_errorInFile("Error: not a number for body_size format accepted is a int"));
		port.push_back(tmpPort);
	}
	for (std::vector<int>::const_iterator itPort = port.begin(); itPort != port.end(); itPort++) {
		bool	isDuplicate = false;
		for (std::vector<int>::const_iterator itPort2 = port.begin(); itPort2 != port.end(); itPort2++) {
			if (*itPort == *itPort2 && isDuplicate)
				return (false);
			if (*itPort == *itPort2)
				isDuplicate = true;
		}
	}
	return (true);
}

bool	Parsing::_extractVctStr(std::vector<std::string> & vctStr, std::string const & line) {
	std::string tmp = line;
	size_t	toFind = tmp.find("=");
	if (toFind != std::string::npos) {
		tmp.erase(0, toFind + 1);
	}
	else
		return (false);
	toFind = tmp.find(",");
	while (toFind != std::string::npos) {
		std::string toPush(tmp, 0, toFind);
		this->_eraseSpace(toPush);
		vctStr.push_back(toPush);
		tmp.erase(0, toFind + 1);
		toFind = tmp.find(",");
	}
	if (tmp.size() > 0) {
		this->_eraseSpace(tmp);
		if (tmp.size() > 0)
			vctStr.push_back(tmp);
	}
	return (true);
}

bool	Parsing::_extractStr(std::string & str, std::string const & line) {
	str = line;
	size_t	toFind = str.find("=");
	if (toFind != std::string::npos) {
		str.erase(0, toFind + 1);
		this->_eraseSpace(str);
		return (true);
	}
	return (false);
}

bool	Parsing::_extractBool(bool & val, std::string const & line) {
	std::string tmp = line;
	size_t	toFind = tmp.find("=");
	if (toFind != std::string::npos) {
		tmp.erase(0, toFind + 1);
		this->_eraseSpace(tmp);
		if (tmp == "true")
			val = true;
		else if (tmp == "false")
			val = false;
		else
			return (false);
		return (true);
	}
	return (false);
}

bool	Parsing::_extractBodySize(int & bodySize, std::string const & line) {
	std::string tmp = line;
	size_t	toFind = tmp.find("=");
	if (toFind != std::string::npos) {
		tmp.erase(0, toFind + 1);
		this->_eraseSpace(tmp);
		try
		{
			bodySize = std::stoi(tmp);
		}
		catch(const std::exception& e)
		{
			return (false);
		}
		return (true);
	}
	return (false);
	return (true);
}

bool 	Parsing::_errorInFile(std::string const & msg) {
	std::cout << msg << std::endl;
	return (false);
}

bool	Parsing::_missingArgForServer(serverSt const & server) {
	if (server.port.size() == 0)
		return (false);
	if (server.adress.size() == 0)
		return (false);
	if (server.serverName.size() == 0)
		return (false);
	if (server.bodySize == -1)
		return (false);
	if (server.cgiExt.size() == 0)
		return (false);
	if (server.cgiDir.size() == 0)
		return (false);
	if (server.root.size() == 0)
		return (false);
	if (server.index.size() == 0)
		return (false);
	if (server.redirectPage.size() == 0)
		return (false);
	return (true);
}

bool	Parsing::_bracketsClose(std::vector<std::string> const & content) {
	int	brackets = 0;
	for (std::vector<std::string>::const_iterator it = content.begin(); it != content.end(); it++) {
		if (it->find("{") != std::string::npos || it->find("}") != std::string::npos)
			brackets++;
	}
	if (brackets % 2 == 0)
		return (true);
	return (false);
}

bool	Parsing::_parseServer(std::vector<std::string> const & content) {
	serverSt server;
	server.bodySize = -1;
	while (!this->_missingArgForServer(server)) {
		for (std::vector<std::string>::const_iterator it = content.begin(); it != content.end(); it++) {
			if (it->find("server {") != std::string::npos)
				continue;
			else if (it->find("port") != std::string::npos) {
				if (!this->_extractPort(server.port, *it))
					return (this->_errorInFile("Error: with port server"));
			}
			else if (it->find("adress") != std::string::npos) {
				if (!this->_extractStr(server.adress, *it))
					return (this->_errorInFile("Error: with adress server"));
			}
			else if (it->find("server_name") != std::string::npos) {
				if (!this->_extractVctStr(server.serverName, *it))
					return (this->_errorInFile("Error: with server_name server"));
			}
			else if (it->find("body_size") != std::string::npos) {
				if (!this->_extractBodySize(server.bodySize, *it))
					return (this->_errorInFile("Error: with body_size server"));
			}
			else if (it->find("cgi_extension") != std::string::npos) {
				if (!this->_extractVctStr(server.cgiExt, *it))
					return (this->_errorInFile("Error: with cgi_extension server"));
			}
			else if (it->find("cgi_dir") != std::string::npos) {
				if (!this->_extractStr(server.cgiDir, *it))
					return (this->_errorInFile("Error: with cgi_dir server"));
			}
			else if (it->find("root") != std::string::npos) {
				if (!this->_extractStr(server.root, *it))
					return (this->_errorInFile("Error: with root server"));
			}
			else if (it->find("error_page") != std::string::npos) {
				if (!this->_extractStr(server.errorPage, *it))
					return (this->_errorInFile("Error: with error_page server"));
			}
			else if (it->find("redirect_page") != std::string::npos) {
				if (!this->_extractStr(server.redirectPage, *it))
					return (this->_errorInFile("Error: with redirect_page server"));
			}
			else if (it->find("index") != std::string::npos) {
				if (!this->_extractStr(server.index, *it))
					return (this->_errorInFile("Error: with index server"));
			}
			else if (it->find("upload_dir") != std::string::npos) {
				if (!this->_extractStr(server.uploadDir, *it))
					return (this->_errorInFile("Error: with upload_dir server"));
			}
			else if (it->find("upload_page") != std::string::npos) {
				if (!this->_extractStr(server.uploadPage, *it))
					return (this->_errorInFile("Error: with upload_page server"));
			}
			else if (it->find("cookie_page_add") != std::string::npos) {
				if (!this->_extractStr(server.cookiePageAdd, *it))
					return (this->_errorInFile("Error: with cookie_page_add server"));
			}
			else if (it->find("cookie_page_delete") != std::string::npos) {
				if (!this->_extractStr(server.cookiePageDelete, *it))
					return (this->_errorInFile("Error: with cookie_page_delete server"));
			}
			else if (it->find("server_location") != std::string::npos) {
				it++;
				std::vector<std::string>::const_iterator itTmp = it;
				serverLocationSt serverLocation;
				for (; itTmp != content.end() && itTmp->find("}") == std::string::npos; itTmp++) {
					if (itTmp->find("uploadable") != std::string::npos) {
						if (!this->_extractBool(serverLocation.uploadable, *itTmp))
							return (this->_errorInFile("Error: with uploadable server_location"));
					}
					else if (itTmp->find("autoindex") != std::string::npos) {
						if (!this->_extractBool(serverLocation.autoindex, *itTmp))
							return (this->_errorInFile("Error: with autoindex server_location"));
					}
					else if (itTmp->find("route") != std::string::npos) {
						if (!this->_extractStr(serverLocation.route, *itTmp))
							return (this->_errorInFile("Error: with route server_location"));
					}
					else if (itTmp->find("allowed_methods") != std::string::npos) {
						if (!this->_extractVctStr(serverLocation.methods, *itTmp))
							return (this->_errorInFile("Error: with allowed_methods server_location"));
					}
					else {
						return (this->_errorInFile("Error: with server_location"));
					}
				}
				while (it != itTmp)
					it++;
				if (serverLocation.route.size() == 0)
					return (this->_errorInFile("Error: not route for server_location"));
				server.serverLocations.push_back(serverLocation);
			}
			else if (it->find("}") == 0) {
				if (server.errorPage.size() == 0)
					server.errorPage = "error.html";
				this->_servers.push_back(server);
				return (true);
			}
		}
	}
	return (true);
}

void	Parsing::_eraseSpace(std::string & str) {
	size_t	toFind = str.find(" ");
	while (toFind == 0) {
		str.erase(0, 1);
		toFind = str.find(" ");
	}
	toFind = str.rfind(" ");
	while (toFind == str.size() - 1) {
		str.erase(str.size() - 1, str.size());
		toFind = str.rfind(" ");
	}
}

void	Parsing::_initPars(std::vector<std::vector<std::string> > & contentServers, std::vector<std::string> const & content) {
	std::vector<std::string> construct;
	for (std::vector<std::string>::const_iterator it = content.begin(); it != content.end(); it++) {
		if (it->find("}") == 0) {
			construct.push_back(*it);
			contentServers.push_back(construct);
			construct.clear();
		}
		else
			construct.push_back(*it);
	}
}