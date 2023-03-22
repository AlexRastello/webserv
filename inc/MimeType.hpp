#pragma once
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <string>

class MimeType
{
	public:
		MimeType();
		virtual~MimeType();
		static std::string					extractMimeType(std::string str);
		std::map<std::string, std::string>&	getMimeType();
	private:
		std::map<std::string, std::string>	_mimeType;
};