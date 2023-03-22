#include "MimeType.hpp"

MimeType::MimeType(){

	this->_mimeType["com"] = "text/plain";
	this->_mimeType["conf"] = "text/plain";
	this->_mimeType["cpp"] = "text/x-c";
	this->_mimeType["doc"] = "application/msword";
	this->_mimeType["exe"] = "application/octet-stream";
	this->_mimeType["gif"] = "image/gif";


	this->_mimeType["text"] = "text/plain";
	this->_mimeType["txt"] = "text/plain";
	this->_mimeType["html"] = "text/html";
	this->_mimeType["asp"] = "text/asp";


	this->_mimeType["bm"] = "image/bmp";
	this->_mimeType["bmp"] = "image/bmp";
	this->_mimeType["ico"] = "image/x-icon";
	this->_mimeType["jpeg"] = "image/jpeg";
	this->_mimeType["jpg"] = "image/jpeg";
	this->_mimeType["png"] = "image/png";

	this->_mimeType["asm"] = "text/x-asm";
	this->_mimeType["c"] = "text/x-c";
	this->_mimeType["c++"] = "text/plain";
	this->_mimeType["class"] = "application/java";
	this->_mimeType["css"] = "text/css";
	this->_mimeType["h"] = "text/x-h";
	this->_mimeType["hpp"] = "text/x-h";
	this->_mimeType["js"] = "application/x-javascript";
	this->_mimeType["py"] = "text/x-script.phyton";
	this->_mimeType["php"] = "application/x-httpd-php";
	this->_mimeType["sh"] = "application/x-sh";
	this->_mimeType["zsh"] = "text/x-scriptzsh";

	this->_mimeType["mp3"] = "audio/mpeg3";

	this->_mimeType["mp4"] = "video/mp4";

	this->_mimeType["tar"] = "application/x-tar";
	this->_mimeType["zip"] = "application/x-zip-compressed";
	this->_mimeType["gz"] = "application/x-gzip";
	this->_mimeType["gzip"] = "application/x-gzip";
}
MimeType::~MimeType(){}

std::string MimeType::extractMimeType(std::string str) {
	std::vector<std::string> vct;
    std::istringstream stream2(str);
    std::string buf;
    while (std::getline(stream2, buf, '.'))
        vct.push_back(buf);
	if (vct.size() > 1)
		return (vct.at(1));
	return ("");
}

std::map<std::string, std::string>&	MimeType::getMimeType() { return (this->_mimeType); }
