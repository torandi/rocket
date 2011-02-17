#include "config.h"
#include <map>
#include <string>
#include <iostream>
#include <cstdio>
#include <exception>
#include <fstream>
#include <istream>

void Config::parse() {
	std::ifstream f(_file.c_str());
	if(f.is_open()) {
		std::string attr,value;
		while(!f.eof()) {
			getline(f,attr,'=');
			getline(f,value);
			if(attr.length()>0) {
				_attr[attr]=value;
			}
		}
	} else {
		printf("Failed to open file %s\n",_file.c_str());
	}
}

Config::Config(std::string file) : _file(file) {
	_attr=std::map<std::string, std::string>();
	parse();	
}

std::string &Config::operator[](const std::string &attr) {
	std::map<std::string, std::string>::iterator it = _attr.find(attr);
	if(it!=_attr.end()) {
		return it->second;
	} else {
		throw std::exception();
	}
}

