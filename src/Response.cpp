#include "Response.hpp"

Response::Response() {}

Response::~Response() {}

void	Response::sendResponse(Request request, int sock) {
    if (request.getCodeRequest() < 400) {
        if (request.getMethod() == "GET") {
            if (request.getCodeRequest() == 301) {
                std::string reply;
                reply += "HTTP/1.1 " + request.getStatusRequest() + "\r\n";
                reply += "Host: " + request.getServerName() + ":" + std::to_string(request.getPort()) + "\r\n";
                reply += "Location : https://www.google.com\r\n";
                reply += "Connection: close\r\n";
                reply += "\r\n";
                send(sock, reply.c_str(), reply.length(), 0);
            }
            else {
                if (request.getContentType() == "cgi") {
                    std::string inputCgi = this->_getInputCgi(request);
                    std::string reply;
                    reply += "HTTP/1.1 200 OK\r\n";
                    reply += "Host: " + request.getServerName() + ":" + std::to_string(request.getPort()) + "\r\n";
                    reply += "Content-Type: text/html\r\n";
                    reply += "Connection: close\r\n";
                    reply += "Content-Length: ";
                    reply += std::to_string(inputCgi.size());
                    reply += "\r\n";
                    reply += "\r\n";
                    reply += inputCgi;
                    send(sock, reply.c_str(), reply.length(), 0);
                }
                else
                    this->_sendToHttp(request, sock);
            }
        }
        else if (request.getMethod() == "POST") {
            if (request.getCodeRequest() == 201) {
                std::time_t t = std::time(0);
                struct stat st;
                if (stat(request.formatRoute(request.getServerParsing().root, request.getServerParsing().uploadDir).c_str(), &st) == -1) {
                    mkdir(request.formatRoute(request.getServerParsing().root, request.getServerParsing().uploadDir).c_str(), 0700);
                }
                std::ofstream file(request.formatRoute(request.getServerParsing().root, request.getServerParsing().uploadDir) + "/" + std::to_string(t) + "_" + request.getFilenameUpload(), std::ios::out | std::ios::binary);
                file << request.getBody();
                file.close();
            }
            this->_sendToHttp(request, sock);
        }
        else if (request.getMethod() == "DELETE") {
            std::ifstream f(request.getRoute());
            if (f.is_open()) {
                f.close();
                remove(request.getRoute().c_str());
                request.setCodeRequest(202);
                request.setStatusRequest("202 Accepted");
                request.setRoute(request.formatRoute(request.getServerParsing().root, request.getServerParsing().uploadPage));
                request.setContentType("text/html");
                this->_sendToHttp(request, sock);
            }
        }
    }
    else
        this->_sendToHttp(request, sock);
}

void    Response::_sendToHttp(Request request, int sock) {
    MimeType mimeType;
    if (!request.getIsAutoIndex()) {
        std::ifstream f(request.getRoute(), std::ios::in | std::ios::binary | std::ios::ate);
        std::streampos size = f.tellg();

        char* other = new char[size];
        f.seekg (0, std::ios::beg);
        f.read(other, size);

        if (size > 200000) {
            std::string convert(other, size);
            long long  sizeSend = 0;

            std::string reply = "HTTP/1.1 200 OK\r\n";
            reply += "Host: " + request.getServerName() + ":" + std::to_string(request.getPort()) + "\r\n";
            reply += "Content-Type: " + mimeType.getMimeType()[MimeType::extractMimeType(request.getRoute())] + "\r\n";
            reply += "Connection: close\r\n";
            reply += "Content-Length: " + std::to_string(size) + "\r\n";
            reply += "\r\n\r\n";
            send(sock, reply.c_str(), reply.size(), 0);
            while (sizeSend < size) {
                size_t  sizeToSend;
                if (sizeSend + 200000 > size) {
                    size_t  newSize = size;
                    sizeToSend = newSize - sizeSend;
                }
                else
                    sizeToSend = 200000;
                sizeSend += 200000;

                std::string strToSend;
                if (sizeToSend == 200000) {
                    strToSend.assign(convert.begin(), convert.begin() + 200000);
                    convert.erase(0, 200000);
                }
                else
                    strToSend = convert;
                send(sock, strToSend.c_str(), strToSend.size(), 0);
            }
        }
        else {
            std::string convert(other, size);
            std::string mimetypeStr = mimeType.getMimeType()[MimeType::extractMimeType(request.getRoute())];
            if (mimetypeStr == "")
                mimetypeStr = "text/plain";
            else if (mimetypeStr == "text/html")
                this->_formatHtml(request, convert);
            std::string reply = "HTTP/1.1 "+ request.getStatusRequest() +"\r\n";
            reply += "Host: " + request.getServerName() + ":" + std::to_string(request.getPort()) + "\r\n";
            reply += "Content-Type: " + mimetypeStr + "\r\n";
            reply += "Connection: close\r\n";
            reply += "Content-Length: " + std::to_string(convert.size()) + "\r\n";
            if (request.getRoute() == request.formatRoute(request.getServerParsing().root, request.getServerParsing().cookiePageAdd))
                reply += "Set-Cookie: webserv=cookie; path=/;\r\n";
            else if (request.getRoute() == request.formatRoute(request.getServerParsing().root, request.getServerParsing().cookiePageDelete))
                reply += "Set-Cookie: webserv=cookie; path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT\r\n";
            reply += "\r\n";
            reply += convert;
            send(sock, reply.c_str(), reply.length(), 0);
        }
        delete[] other;
        f.close();
    }
    else {
        std::string html;
        html += "<!DOCTYPE html>";
        html += "<html lang=\"fr\">";
        html += "<head>";
        html += "<meta charset=\"UTF-8\">";
        html += "<link rel=\"icon\" type=\"image/png\" href=\"img/favicon.png\" />";
        html += "<link rel=\"stylesheet\" href=\"style.css\">";
        html += "<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">";
        html += "<link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>";
        html += "<link href=\"https://fonts.googleapis.com/css2?family=Audiowide&family=Roboto&display=swap\" rel=\"stylesheet\">";
        html += "<title>Webserv</title>";
        html += "</head>";
        html += "<body>";
        html += "<div class=\"content\">";
        html += "<h1 class=\"titleh1\">AUTOINDEX</h1>";
        html += "<nav class=\"nav-content-listing\">";
        html += "<ul>";
        html += "INSERT";
        html += "</ul>";
        html += "</nav>";
        html += "</div>";
        html += "</body>";
        html += "</html>";
        this->_formatHtml(request, html);

        std::string reply = "HTTP/1.1 200 OK\r\n";
        reply += "Host: " + request.getServerName() + ":" + std::to_string(request.getPort()) + "\r\n";
        reply += "Content-Type: text/html\r\n";
        reply += "Connection: close\r\n";
        reply += "Content-Length: " + std::to_string(html.size()) + "\r\n";
        reply += "\r\n";
        reply += html;
        send(sock, reply.c_str(), reply.length(), 0);
    }
}

void Response::_formatHtml(Request request, std::string &html) {
    if (request.getRoute() == request.formatRoute(request.getServerParsing().root, request.getServerParsing().uploadPage)) {
        size_t  toFindPort = html.find("PORT");
        if (toFindPort != std::string::npos) {
            html.erase(toFindPort, 4);
            html.insert(toFindPort, std::to_string(request.getPort()));
        }
        DIR *dir = opendir(request.formatRoute(request.getServerParsing().root, request.getServerParsing().uploadDir).c_str());
        if (dir) {
            size_t  toFind = html.find("LOAD_FILE");
            if (toFind != std::string::npos) {
                html.erase(toFind, 9);
                dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    std::string tmp(entry->d_name);
                    if (tmp == "." || tmp == "..")
                        continue;
                    std::string insert = "<div class=\"div-link\"><p class=\"p-link\">" + tmp + "</p><a " "href=\"upload/" + tmp + "\" download class=\"download-file\" >Download</a><button class=delete-file value=" + tmp + ">Delete</button></div>";
                    html.insert(toFind, insert);
                }
            }
            free(dir);
        }
    }
    else if (request.getRoute() == request.formatRoute(request.getServerParsing().root, request.getServerParsing().errorPage)) {
        int countDir = 0;
        {
            std::string tmp = request.getRouteNotFound();
            size_t  toFind = tmp.find('/');
            if (toFind == 0) {
                tmp.erase(0, 1);
                toFind = tmp.find('/');
            }
            while (toFind != std::string::npos) {
                tmp.erase(0, toFind + 1);
                toFind = tmp.find('/');
                countDir++;
            }
        }
        {
            size_t  toFind = html.find("CHANGE");
            while (toFind != std::string::npos) {
                html.erase(toFind, 6);
                html.insert(toFind, request.getStatusRequest());
                toFind = html.find("CHANGE");
            }
        }
        {
            std::string favicon = "img/favicon.png";
            size_t toFind = html.find("img/favicon.png");
            if (toFind != std::string::npos) {
                html.erase(toFind, favicon.size());
                int i = 0;
                while (i < countDir) {
                    favicon.insert(0, "../");
                    i++;
                }
                html.insert(toFind, favicon);
            }
        }
        {
            std::string style = "style.css";
            size_t toFind = html.find("style.css");
            if (toFind != std::string::npos) {
                html.erase(toFind, style.size());
                int i = 0;
                while (i < countDir) {
                    style.insert(0, "../");
                    i++;
                }
                html.insert(toFind, style);
            }
        }
    }
    else if (request.getIsAutoIndex()) {
        dirent *entry;
        std::string tmpRoute = request.getRoute();
        DIR *dir = opendir(request.getRoute().c_str());
        if (dir) {
            int countDir = 0;
            std::string tmp = request.getRoute();
            size_t  toFindDir = tmp.find('/');
            if (tmp.rfind("/") == tmp.size() - 1 && tmp != "/")
                tmp.erase(tmp.rfind("/"));
            while (toFindDir != std::string::npos) {
                tmp.erase(0, toFindDir + 1);
                toFindDir = tmp.find('/');
                countDir++;
            }
            size_t toFindCss = html.find("style.css");
            if (toFindCss != std::string::npos) {
                int i = 0;
                while (i < countDir) {
                    html.insert(toFindCss, "../");
                    i++;
                }
            }
            size_t toFindImg = html.find("img/favicon.png");
            if (toFindImg != std::string::npos) {
                int i = 0;
                while (i < countDir) {
                    html.insert(toFindImg, "../");
                    i++;
                }
            }
            size_t  toFindInsert = html.find("INSERT");
            if (toFindInsert != std::string::npos) {
                html.erase(toFindInsert, 6);
                while ((entry = readdir(dir)) != NULL) {
                    std::string nameFile(entry->d_name);
                    if (nameFile == ".")
                        continue;
                    else if (nameFile == "..") {
                        std::string hrefFile;
                        std::vector<std::string> vctRoute = request.extractRouteFromRequest(tmpRoute);
                        for (std::vector<std::string>::const_iterator it = vctRoute.begin(); it != vctRoute.end() - 1; it++) {
                            if (it != vctRoute.begin())
                                hrefFile += *it;
                        }
                        hrefFile += "..";
                        std::string insert = "<li><a href=\"" + hrefFile + "\">../</a></li>";
                        html.insert(toFindInsert, insert);
                    }
                    else {
                        std::string hrefFile = request.formatRoute(tmpRoute, nameFile);
                        DIR *testDir = opendir(hrefFile.c_str());
                        if (testDir)
                            nameFile.insert(nameFile.size(), "/");
                        size_t toFindRoot = hrefFile.find(request.formatRoute(request.getServerParsing().root));
                        if (toFindRoot != std::string::npos)
                            hrefFile.erase(0, request.formatRoute(request.getServerParsing().root).size() + 1);
                        hrefFile.insert(0, "/");
                        std::string insert = "<li><a href=\"" + hrefFile + "\">" + nameFile + "</a></li>";
                        html.insert(toFindInsert, insert);
                        free(testDir);
                    }
                }
            }
            free(dir);
        }
    }

}

std::string Response::_getContent(std::string path) {
	std::fstream file(path);
	std::string	tmp;
	std::string	content;
	while (file.good()) {
		std::getline(file, tmp);
		content += tmp;
	}
	return (content);
}

std::string	Response::_getInputCgi(Request request) {
    std::string inputCgi;
    int fd[2];
    char    **tab = new char*[3];
    std::vector<std::string> data;
    std::ifstream f(request.getRoute());
    if (f) {
        f.close();
        if (MimeType::extractMimeType(request.getRoute()) == "py")
            tab[0] =  strdup("/usr/bin/python");
        else if (MimeType::extractMimeType(request.getRoute()) == "php")
            tab[0] =  strdup("/usr/bin/php");
        tab[1] = strdup(request.getRoute().c_str());
        tab[2] = NULL;
        pipe(fd);
        pid_t pid = fork();
        if (pid == 0) {
            dup2(fd[1], 1);
            close(fd[1]);
            close(fd[0]);
            if (access(request.getRoute().c_str(), F_OK) == -1)
                exit(1);
            if (access(tab[0], F_OK) == -1)
                exit(1);
            execve(tab[0], tab, NULL);
        }
        waitpid(-1, NULL, 0);
        close(fd[1]);
        char c;
        while (read(fd[0], &c, 1)) {
            inputCgi += c;
        }
        close(fd[0]);
    }
    delete [] tab;
    return (inputCgi);
}
