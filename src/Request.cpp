#include "Request.hpp"

Request::Request(serverSt const & serverParsing, int const & port) {
    this->_isEraseFirstBound = false;
    this->_isAutoIndex = false;
    this->_codeRequest = 0;
    this->_contentLength = 0;
    this->_request.clear();
    this->_inputCgi.clear();
    this->_route.clear();
    this->_routeNotFound.clear();
    this->_filenameUpload.clear();
    this->_method.clear();
    this->_body.clear();
    this->_contentType.clear();
    this->_statusRequest.clear();
    this->_webkit.clear();
    this->_serverName.clear();
    this->_serverParsing = serverParsing;
    this->_port = port;
}
Request::~Request() {}

//Setters
void		                Request::setRoute(std::string const & route) { this->_route = route; }
void						Request::setCodeRequest(int const & codeRequest) { this->_codeRequest = codeRequest; }
void						Request::setStatusRequest(std::string const & statusRequest) { this->_statusRequest = statusRequest; }
void						Request::setContentType(std::string const & contentType) { this->_contentType = contentType; }

//Getters
int const &							Request::getPort() const { return (this->_port); }
int const &							Request::getCodeRequest() const { return (this->_codeRequest); }
bool const &                        Request::getIsAutoIndex() const { return (this->_isAutoIndex); }
size_t      		                Request::getContentLength(void) const { return (this->_contentLength); }
std::string const &	                Request::getStatusRequest() const { return (this->_statusRequest); }
std::string const &	                Request::getRoute() const { return (this->_route); }
std::string const &	                Request::getRouteNotFound() const { return (this->_routeNotFound); }
std::string const &	                Request::getFilenameUpload() const { return (this->_filenameUpload); }
std::string const &	                Request::getMethod(void) const { return (this->_method); }
std::string const &	                Request::getBody(void) const { return (this->_body); }
std::string const &	                Request::getContentType() const { return (this->_contentType); }
std::string const &	                Request::getInputCgi() const { return (this->_inputCgi); }
std::string const &					Request::getErrorRequest() const { return (this->_statusRequest); }
std::string const &					Request::getServerName() const { return (this->_serverName); }
serverSt const &					Request::getServerParsing() const { return (this->_serverParsing); }


std::string		Request::formatRoute(std::string const & s1, std::string const & s2) {
	std::string tmp1 = s1;
	std::string tmp2 = s2;
	if (tmp1.find("/") == 0)
		tmp1.erase(0, 1);
	if (tmp2.find("/") == 0) {
		return (tmp1 + tmp2);
	}
	return (tmp1 + "/" + tmp2);
}

std::string		Request::formatRoute(std::string const & s) {
	std::string tmp = s;
	if (tmp.find("/") == 0)
		tmp.erase(0, 1);
	return (tmp);
}

void                        Request::formatBody() {
    this->_filenameUpload = this->_extractFilenameUpload(this->_getVctRequest(this->_body));
    MimeType mimeType;
    if (this->_codeRequest == 200) {
        if (this->_filenameUpload == "")
            this->_successInRequest(200, "200 OK", this->_route, "html");
        else if (mimeType.getMimeType()[MimeType::extractMimeType(this->_filenameUpload)] == "")
            this->_errorInRequest(415, "415 Unsupported Media Type", this->_route);
        else if (this->_contentLength > this->_serverParsing.bodySize)
            this->_errorInRequest(413, "413 Payload Too Large", this->_route);
        else {
            this->_body.erase(0, this->_body.find("Content-Type"));
            this->_body.erase(0, this->_body.find('\n') + 1);
            this->_body.erase(0, this->_body.find('\n') + 1);
            this->_body.erase(this->_body.find(this->_webkit + "--"), this->_body.size() - 2);
            this->_successInRequest(201, "201 Created", this->_route, "text/html");
        }
    }

}

void		                Request::insertRequestInBody(std::string const & request) {
    this->_body += request;
}

void		                Request::parseRequest(std::string const & request) {
    bool    isAllowedRoute = false;
    bool    isAllowedMethod = false;
    MimeType mimetype;
    std::vector<std::string> vctMethod = this->_getVctRequestMethod(request);
    if (vctMethod.size() == 3) {
        if (vctMethod.at(1).find("/") == std::string::npos || vctMethod.at(2).find("HTTP/") == std::string::npos) {
            this->_errorInRequest(400, "400 Bad Request", this->formatRoute(this->_serverParsing.root + this->_serverParsing.errorPage));
            return;
        }
		this->_method = vctMethod.at(0);
        std::string versionHttp = vctMethod.at(2);
        versionHttp.erase(8, 2);
        if (versionHttp != "HTTP/1.1") {
            this->_errorInRequest(505, "505 HTTP Version Not Supported", this->formatRoute(vctMethod.at(1)));
        }
        else {
            this->_extractBodySizeAndWebkit(request);
            if (this->_contentLength > this->_serverParsing.bodySize) {
                this->_errorInRequest(413, "413 Payload Too Large", this->formatRoute(vctMethod.at(1)));
                return;
            }
            bool    isAllowedServername = false;
            for (std::vector<std::string>::const_iterator it = this->_serverParsing.serverName.begin(); it != this->_serverParsing.serverName.end(); it++) {
                if (this->_serverName == *it) {
                    isAllowedServername = true;
                    this->_serverName = *it;
                    break;
                }
            }
            if (!isAllowedServername || this->_serverName.size() == 0) {
                this->_errorInRequest(403, "403 Forbidden", this->formatRoute(vctMethod.at(1)));
                return;
            }
            std::string tmpRoute = this->formatRoute(this->_serverParsing.root, vctMethod.at(1));
            struct stat s;
            if (stat(tmpRoute.c_str(), &s) == 0) {
                if (s.st_mode & S_IFDIR) {
                     std::string tmpSub = vctMethod.at(1);
                    if (tmpSub.rfind("/") == tmpSub.size() - 1 && tmpSub != "/")
                        tmpSub.erase(tmpSub.rfind("/"));
                    std::vector<serverLocationSt>   vctLocation = this->_serverParsing.serverLocations;
                    for (std::vector<serverLocationSt>::iterator it = vctLocation.begin(); it != vctLocation.end(); it++) {
                        if (it->route == tmpSub) {
                            for (std::vector<std::string>::iterator itMethod = it->methods.begin(); itMethod != it->methods.end(); itMethod++) {
                                if (*itMethod == this->_method) {
                                    isAllowedMethod = true;
                                    break;
                                }
                            }
                            if (it->autoindex && tmpSub != "/") {
                                this->_isAutoIndex = true;
                                this->_successInRequest(200, "200 OK", tmpRoute, mimetype.getMimeType()[MimeType::extractMimeType(tmpRoute)]);
                                isAllowedRoute = true;
                            }
                            else if (tmpSub == "/") {
                                this->_successInRequest(200, "200 OK", this->formatRoute(this->_serverParsing.root, this->_serverParsing.index), mimetype.getMimeType()[MimeType::extractMimeType(tmpRoute)]);
                                isAllowedRoute = true;
                            }
                            else {
                                this->_errorInRequest(404, "404 Not Found", tmpRoute);
                            }
                            if (this->_method == "POST" && isAllowedMethod) {
                                if (!it->uploadable)
                                    this->_errorInRequest(403, "403 Forbidden", tmpRoute);
                            }
                            break;
                        }
                    }
                }
                else if (s.st_mode & S_IFREG) {
                    std::vector<std::string> vctReqRoute = this->extractRouteFromRequest(vctMethod.at(1));
                    std::string reqRoute;
                    for (std::vector<std::string>::iterator it = vctReqRoute.begin(); it != vctReqRoute.end() - 1; it++)
                        reqRoute += *it;
                    if (reqRoute.rfind("/") == reqRoute.size() - 1 && reqRoute != "/")
                        reqRoute.erase(reqRoute.rfind("/"));

                    std::vector<serverLocationSt>   vctLocation = this->_serverParsing.serverLocations;
                    for (std::vector<serverLocationSt>::iterator it = vctLocation.begin(); it != vctLocation.end(); it++) {
                        if (it->route == reqRoute) {
                            for (std::vector<std::string>::iterator itMethod = it->methods.begin(); itMethod != it->methods.end(); itMethod++) {
                                if (*itMethod == this->_method) {
                                    isAllowedMethod = true;
                                    break;
                                }
                            }
                            if (it->route == this->_serverParsing.cgiDir && mimetype.getMimeType()[MimeType::extractMimeType(tmpRoute)] != "text/html" && mimetype.getMimeType()[MimeType::extractMimeType(tmpRoute)] != "text/css") {
                                bool    isAllowedExt = false;
                                for (std::vector<std::string>::iterator itExt = this->_serverParsing.cgiExt.begin(); itExt != this->_serverParsing.cgiExt.end(); itExt++) {
                                    if (*itExt == MimeType::extractMimeType(tmpRoute) && (MimeType::extractMimeType(tmpRoute) == "php" ||  MimeType::extractMimeType(tmpRoute) == "py")) {
                                        isAllowedExt = true;
                                        break;
                                    }
                                }
                                if (isAllowedExt)
                                    this->_successInRequest(200, "200 OK", tmpRoute, "cgi");
                                else {
                                    this->_errorInRequest(415, "415 Unsupported Media Type", tmpRoute);
                                    return;
                                }
                            }
                            else if (tmpRoute.find(this->_serverParsing.redirectPage) != std::string::npos)
                                this->_successInRequest(301, "301 Moved Permanently", tmpRoute, mimetype.getMimeType()[MimeType::extractMimeType(tmpRoute)]);
                            else
                                this->_successInRequest(200, "200 OK", tmpRoute, mimetype.getMimeType()[MimeType::extractMimeType(tmpRoute)]);
                            isAllowedRoute = true;
                            if (this->_method == "POST" && isAllowedMethod) {
                                if (!it->uploadable)
                                    this->_errorInRequest(403, "403 Forbidden", tmpRoute);
                            }
                            break;
                        }
                    }
                }
            }
            if (!isAllowedRoute)
                this->_errorInRequest(404, "404 Not Found", tmpRoute);
            else if (!isAllowedMethod) {
                if (this->_method != "GET" && this->_method != "POST" && this->_method != "DELETE")
                    this->_errorInRequest(501, "501 Not Implemented", tmpRoute);
                else
                    this->_errorInRequest(405, "405 Method Not Allowed", tmpRoute);
            }
        }
    }
    else {
        this->_errorInRequest(400, "400 Bad Request", this->formatRoute(this->_serverParsing.root + this->_serverParsing.errorPage));
    }
}

void		                Request::clear() {
    this->_isEraseFirstBound = false;
    this->_isAutoIndex = false;
    this->_codeRequest = 0;
    this->_contentLength = 0;
    this->_request.clear();
    this->_inputCgi.clear();
    this->_route.clear();
    this->_routeNotFound.clear();
    this->_filenameUpload.clear();
    this->_method.clear();
    this->_body.clear();
    this->_contentType.clear();
    this->_statusRequest.clear();
    this->_webkit.clear();
    this->_serverName.clear();
}

// //Private functions

void                        Request::_successInRequest(int const & codeRequest, std::string const & statusRequest, std::string const & route, std::string const & contentType) {
    this->_codeRequest = codeRequest;
    this->_statusRequest = statusRequest;
    this->_route = this->formatRoute(route);
    this->_contentType = contentType;
}

void                        Request::_errorInRequest(int const & codeRequest, std::string const & statusRequest, std::string const & routeNotFound) {
    this->_routeNotFound = this->formatRoute(routeNotFound);
    this->_codeRequest = codeRequest;
    this->_statusRequest = statusRequest;
    this->_route = this->formatRoute(this->_serverParsing.root, this->_serverParsing.errorPage);
    this->_contentType = "text/html";
}

std::vector<std::string>    Request::extractRouteFromRequest(std::string const & requestFirstLine) {
    std::string routeTmp = requestFirstLine;
    std::vector<std::string> routeRes;
    size_t  findRoute = routeTmp.find('/');
    while (findRoute != std::string::npos) {
        std::string tmp = routeTmp.substr(0, findRoute + 1);
        if (tmp.rfind("/") != tmp.size() - 1 || tmp.find("/") == std::string::npos) {
            tmp.insert(tmp.size() - 1, "/");
        }
        routeRes.push_back(tmp);
        routeTmp.erase(0, findRoute + 1);
        findRoute = routeTmp.find('/');
    }
    if (routeTmp != "")
        routeRes.push_back(routeTmp);
    return (routeRes);
}

std::vector<std::string>    Request::_extractRouteFromLocation(std::string const & locationRoute) {
    std::string routeTmp = locationRoute;
    std::vector<std::string> routeRes;
    size_t  findRoute = routeTmp.find('/');
    while (findRoute != std::string::npos) {
        std::string tmp = routeTmp.substr(0, findRoute + 1);
        if (tmp.rfind("/") != tmp.size() - 1 || tmp.find("/") == std::string::npos) {
            tmp.insert(tmp.size() - 1, "/");
        }
        routeRes.push_back(tmp);
        routeTmp.erase(0, findRoute + 1);
        findRoute = routeTmp.find('/');
    }
    if (routeTmp != "") {
        if (routeTmp.rfind("/") != routeTmp.size() - 1 || routeTmp.find("/") == std::string::npos) {
            routeTmp.insert(routeTmp.size(), "/");
        }
        routeRes.push_back(routeTmp);
    }
    return (routeRes);
}

void                                Request::_extractBodySizeAndWebkit(std::string const & request) {
    std::vector<std::string> vctRequest = this->_getVctRequest(request);
    std::vector<std::string>::iterator it;
    for (it = vctRequest.begin(); it != vctRequest.end(); it++) {
        if (it->find("Content-Length") != std::string::npos) {
            size_t  toFind = it->find(' ');
            if (toFind != std::string::npos) {
                std::string tmp = *it;
                tmp.erase(0, toFind + 1);
                this->_contentLength = std::atoi(tmp.c_str());
            }
        }
        else if (it->find("Content-Type: multipart/form-data; boundary=") != std::string::npos) {
            this->_webkit = *it;
            this->_webkit.erase(0, this->_webkit.find("=") + 1);
            this->_webkit.erase(this->_webkit.size() - 1);
            this->_webkit.insert(0, "--");
        }
        else if (it->find("Host:") != std::string::npos) {
            this->_serverName = *it;
            this->_serverName.erase(0, 6);
            this->_serverName.erase(this->_serverName.find(":"), std::to_string(this->_port).size() + 3);
        }
    }
}

std::string                 Request::_extractFilenameUpload(std::vector<std::string> const & vct) {
    std::string filename;

    for (std::vector<std::string>::const_iterator it = vct.begin(); it != vct.end(); it++) {
        std::vector<std::string> tmp = this->_getVctRequest(*it);
        for (std::vector<std::string>::iterator itTmp = tmp.begin(); itTmp != tmp.end(); itTmp++) {
            if (itTmp->find("filename=") != std::string::npos) {
                filename = *itTmp;
                filename.erase(0, itTmp->find("filename=") + 10);
                filename.erase(filename.end() - 2, filename.end());
                size_t  toFind = filename.find(" ");
                while (toFind != std::string::npos) {
                    filename.erase(toFind, 1);
                    toFind = filename.find(" ");
                }
                return (filename);
            }
        }
    }
    return (filename);
}

std::vector<std::string>	Request::_getVctRequest(std::string const & request) {
    std::istringstream stream(request);
    std::vector<std::string> vct;
    std::string buf;
    while (std::getline(stream, buf, '\n')) {
        vct.push_back(buf);
    }
    return (vct);
}

std::vector<std::string>	Request::_getVctRequestMethod(std::string const & request) {
    std::istringstream stream(request);
    std::string split;
    std::getline(stream, split, '\n');
    std::vector<std::string> vct;
    std::istringstream stream2(split);
    std::string buf;
    while (std::getline(stream2, buf, ' ')) {
        vct.push_back(buf);
    }
    return (vct);
}