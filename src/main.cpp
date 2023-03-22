#include <pthread.h>
#include "Parsing.hpp"
#include "Server.hpp"

std::string	pathFile;

struct	argRoutine {
	std::string	pathFile;
	std::string	adress;
	serverSt	serverParsing;
	int			port;
};


void	*routine(void *arg) {
	argRoutine	*args = reinterpret_cast<argRoutine*>(arg);
	Server server(args->serverParsing, args->port);
	server.initServ();
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
	if (argc > 2) {
		std::cout << "Error: with argv\n";
		return (1);
	}
	Parsing pars;
	if (argc == 2) {
		if (!pars.setupParsing(argv[1]))
			return (1);
		else
			pathFile = argv[1];
	}
	else {
		if (!pars.setupParsing("webserv.conf"))
			return (1);
		else
			pathFile = "webserv.conf";
	}
	if (pars.getServers().size() == 1 && pars.getServers().at(0).port.size() == 1) {
		Server server(pars.getServers().at(0), pars.getServers().at(0).port.at(0));
		std::cout << "\033[1;37mSERVER SETUP => \033[0m" << "\033[1;33m" << pars.getServers().at(0).adress << ":" << pars.getServers().at(0).port.at(0) << "\033[0m" << std::endl;
		server.initServ();
	}
	else {
		std::vector<pthread_t> vctPthread;
		std::vector<argRoutine> vctArgRoutine;
		for (std::vector<serverSt>::const_iterator itServ = pars.getServers().begin(); itServ != pars.getServers().end(); itServ++) {
			for (std::vector<int>::const_iterator itPort = itServ->port.begin(); itPort != itServ->port.end(); itPort++) {
				argRoutine args;
				args.port = *itPort;
				args.pathFile = pathFile;
				args.serverParsing = *itServ;
				args.adress = itServ->adress;
				vctArgRoutine.push_back(args);
			}
		}
		for (std::vector<argRoutine>::iterator itArg = vctArgRoutine.begin(); itArg != vctArgRoutine.end(); itArg++) {
			pthread_t id;
			std::cout << "\033[1;37mSERVER SETUP => \033[0m" << "\033[1;33m" << itArg->serverParsing.adress << ":" << itArg->port << "\033[0m" << std::endl;
			pthread_create(&id, NULL, routine, (void *) &(*itArg));
			vctPthread.push_back(id);
		}
		for (std::vector<pthread_t>::iterator it = vctPthread.begin(); it != vctPthread.end(); it++)
			pthread_join(*it, NULL);
	}
	return (0);
}