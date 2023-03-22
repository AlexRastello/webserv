#include "Server.hpp"

Server::Server(serverSt parsingServer, int port): _parsingServer(parsingServer), _port(port) {
    for (int i = 0; i < SOMAXCONN; i++)
        this->_clients[i].setFd(0);
}
Server::~Server() {}

void    Server::initServ() {
    if (this->_mainSocket())
        this->_runEventLoop();
}

void    Server::_handleRecv(int sock, Request &request, std::vector<struct kevent> &vct, bool &isRecv) {
    if (request.getRoute().size() == 0) {
        char buffer[10000];
        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        std::string httpRequest(buffer, bytesReceived);

        if (bytesReceived < 0) {
            vct.push_back((struct kevent) {});
            EV_SET(&vct.back(), sock, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
            return;
        }
        // std::cout << httpRequest << std::endl;
        if (httpRequest.find("------WebKitFormBoundary") != std::string::npos) {
            std::string tmp = httpRequest;
            tmp.erase(0, tmp.find("------WebKitFormBoundary"));            
            request.insertRequestInBody(tmp);
            request.parseRequest(httpRequest);

            if (request.getBody().size() >= request.getContentLength()) {
                request.formatBody();
                vct.push_back((struct kevent) {});
                EV_SET(&vct.back(), sock, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
                vct.push_back((struct kevent) {});
                EV_SET(&vct.back(), sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
                isRecv = false;
            }
            else {
                vct.push_back((struct kevent) {});
                EV_SET(&vct.back(), sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                isRecv = true;
            }
        }
        else {
            request.parseRequest(httpRequest);
            if (request.getContentLength() == 0) {
                isRecv = false;
                vct.push_back((struct kevent) {});
                EV_SET(&vct.back(), sock, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
                vct.push_back((struct kevent) {});
                EV_SET(&vct.back(), sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
            }
            else {
                vct.push_back((struct kevent) {});
                EV_SET(&vct.back(), sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                isRecv = true;
            }
        }
    }
    else if (request.getRoute().size() > 0) {
        char bufferPost[200000];
        int bytesReceivedPost = recv(sock, bufferPost, sizeof(bufferPost), 0);
        if (bytesReceivedPost < 0) {
            return;
        }
        std::string httpRequestPost(bufferPost, bytesReceivedPost);
        // std::cout << bytesReceivedPost << std::endl;
        // std::cout << httpRequestPost << std::endl;
        request.insertRequestInBody(httpRequestPost);

        if (request.getBody().size() >= request.getContentLength()) {
            request.formatBody();
            vct.push_back((struct kevent) {});
            EV_SET(&vct.back(), sock, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
            vct.push_back((struct kevent) {});
            EV_SET(&vct.back(), sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
            isRecv = false;
        }
        else {
            vct.push_back((struct kevent) {});
            EV_SET(&vct.back(), sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
            isRecv = true;
        }
    }
    else {
        vct.push_back((struct kevent) {});
        EV_SET(&vct.back(), sock, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
        vct.push_back((struct kevent) {});
        EV_SET(&vct.back(), sock, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
        isRecv = false;
    }
}

void    Server::_runEventLoop() {
    bool isRecv = false;
    int kq = kqueue();
    if (kq < 0) {
        std::cout << "Failed to create kqueue" << std::endl;
        return;
    }
    struct sockaddr_storage addr;
    socklen_t socklen = sizeof(addr);
    struct kevent evLst[SOMAXCONN];

    std::vector<struct kevent> vct;
    vct.push_back((struct kevent) {});
    EV_SET(&vct.back(), this->_listener, EVFILT_READ, EV_ADD, 0, 0, NULL);

    Request request(this->_parsingServer, this->_port);
    while (true) {
        int nev = kevent(kq, vct.data(), vct.size(), evLst, SOMAXCONN, NULL);
        if (nev < 0) {
            std::cout << "Failed to wait for event" << std::endl;
        }
        vct.shrink_to_fit();
        vct.clear();
        for (int i = 0; i < nev; i++) {
            if (evLst[i].flags & EV_EOF) {
                vct.push_back((struct kevent) {});
                EV_SET(&vct.back(), evLst[i].ident, EVFILT_READ, EV_DELETE | EV_DISABLE, 0, 0, NULL);
                isRecv = false;
                request.clear();
            }
            else if (evLst[i].ident == this->_listener && request.getRoute().size() == 0) {
                int sock = accept(this->_listener, (struct sockaddr *) &addr, &socklen);
                fcntl(sock, F_SETFL, O_NONBLOCK);                
                if (this->_connAdd(sock) != 0){
                    std::cout << "Failed to accept connection" << std::endl;
                    close(sock);
                }
                else {
                    vct.push_back((struct kevent) {});
                    EV_SET(&vct.back(), sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    isRecv = true;
                    vct.push_back((struct kevent) {});
                    EV_SET(&vct.back(), sock, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, 6000, NULL);
                }
            }
            else if (evLst[i].filter == EVFILT_READ && isRecv) {
                this->_handleRecv(evLst[i].ident, request, vct, isRecv);
            }
            else if (evLst[i].filter == EVFILT_WRITE && !isRecv) {
                isRecv = true;
                Response response;
                response.sendResponse(request, evLst[i].ident);
                request.clear();

                vct.push_back((struct kevent) {});
                EV_SET(&vct.back(), evLst[i].ident, EVFILT_WRITE, EV_DELETE | EV_DISABLE, 0, 0, NULL);
                this->_connDel(evLst[i].ident);
            }
            else if (evLst[i].filter == EVFILT_TIMER) {
                isRecv = false;
                request.clear();
                this->_connDel(evLst[i].ident);
            }
        }
    }
}

bool    Server::_mainSocket() {
    this->_listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (this->_listener < 0)
        std::cout << "Failed to create socket" << std::endl;
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->_port);
    inet_aton(this->_parsingServer.adress.c_str(), (in_addr *)&addr.sin_addr.s_addr);
    int	opt = 1;
	if (setsockopt(this->_listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		std::cout << "Error setsocketopt reuseaddr" << std::endl;
        return (false);
    }
	if (setsockopt(this->_listener, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
		std::cout << "Error setsocketopt reuseport" << std::endl;
        return (false);
    }
    if (bind(this->_listener, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        std::cout << "Failed to bind socket" << std::endl;
        return (false);
    }
    if (listen(this->_listener, SOMAXCONN) < 0) {
        std::cout << "Failed to listen on socket" << std::endl;
        return (false);
    }
    return (true);
}

int Server::_getConn(int fd) {
    for (int i = 0; i < SOMAXCONN; i++)
        if (this->_clients[i].getFd() == fd)
            return i;
    return -1;
}

int Server::_connAdd(int fd) {
    if (fd < 1)
        return -1;
    int i = this->_getConn(0);
    if (i == -1)
        return -1;
    this->_clients[i].setFd(fd);
    return 0;
}

int Server::_connDel(int fd) {
    if (fd < 1)
        return -1;
    int i = this->_getConn(fd);
    if (i == -1)
        return -1;
    this->_clients[i].setFd(0);
    return close(fd);
}