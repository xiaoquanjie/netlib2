#ifndef M_BASE_CONFIGREADER_INCLUDE
#define M_BASE_CONFIGREADER_INCLUDE

#include "base/config.hpp"
#include <string>
#include <fstream>
#include <map>
#include <stdlib.h>

M_BASE_NAMESPACE_BEGIN

class config_reader{
public:
	config_reader();

	~config_reader();

	bool loadFile(const std::string& file);

	std::string getString(const std::string& key)const;

	std::string getString(const std::string& key, const std::string& defaultval)const;

	int getInt(const std::string& key)const;

	int getInt(const std::string& key, int defaultval)const;

	short getShort(const std::string& key)const;

	short getShort(const std::string& key, short defaultval)const;

	char getChar(const std::string& key)const;

	char getChar(const std::string& key, char defaultval)const;

	bool setString(const std::string& key, const std::string& value);

	bool save(const std::string& file);

protected:
	std::map<std::string, std::string>
		_config_map;
};

inline config_reader::config_reader() {

}

inline config_reader::~config_reader() {

}

inline bool config_reader::loadFile(const std::string& file) {
	_config_map.clear();
	std::ifstream ifs(file.c_str());
	char line_content[1024];
	std::string content;
	while (ifs) {
		ifs.getline(line_content, 1024);
		content = line_content;
		for (std::string::iterator iter = content.begin(); 
			iter != content.end(); ){
			if ((*iter) == ' ' || (*iter) == '\t'){
				iter = content.erase(iter);
			}
			else if ((*iter) == ';' || (*iter) == '#'){
				iter = content.erase(iter, content.end());
			}
			else {
				++iter;
			}
		}
		int pos = content.find('=');
		if (pos != std::string::npos){
			_config_map[content.substr(0, pos)] = content.substr(pos + 1, content.size());
		}
	}
	ifs.close();
}

inline std::string config_reader::getString(const std::string& key)const {
	return getString(key, "");
}

inline std::string config_reader::getString(const std::string& key, const std::string& defaultval)const {
	std::map<std::string, std::string>::const_iterator iter = _config_map.find(key);
	if (iter != _config_map.end())
		return iter->second;
	else
		return defaultval;
}

inline int config_reader::getInt(const std::string& key)const {
	return getInt(key, 0);
}

inline int config_reader::getInt(const std::string& key, int defaultval)const {
	std::map<std::string, std::string>::const_iterator iter = _config_map.find(key);
	if (iter != _config_map.end())
		return atoi(iter->second.c_str());
	else
		return defaultval;
}

inline short config_reader::getShort(const std::string& key)const {
	return getShort(key, 0);
}

inline short config_reader::getShort(const std::string& key, short defaultval)const {
	std::map<std::string, std::string>::const_iterator iter = _config_map.find(key);
	if (iter != _config_map.end())
		return (short)atoi(iter->second.c_str());
	else
		return defaultval;
}

inline char config_reader::getChar(const std::string& key)const {
	return getChar(key, 0);
}

inline char config_reader::getChar(const std::string& key, char defaultval)const {
	std::map<std::string, std::string>::const_iterator iter = _config_map.find(key);
	if (iter != _config_map.end())
		return (char)atoi(iter->second.c_str());
	else
		return defaultval;
}

inline bool config_reader::setString(const std::string& key, const std::string& value) {
	_config_map[key] = value;
}

inline bool config_reader::save(const std::string& file) {
	try {
		std::ofstream ofs(file.c_str());
		for (auto iter = _config_map.begin(); iter != _config_map.end(); ++iter) {
			ofs << iter->first << " = " << iter->second << std::endl;
		}
		ofs.close();
		return true;
	}
	catch (...) {
		return false;
	}
}

M_BASE_NAMESPACE_END
#endif