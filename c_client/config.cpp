#include "config.h"
#include <map>
#include <string>
#include <iostream>
#include <cstdio>
#include <exception>
#include <fstream>
#include <istream>

bool Config::parse() {
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
		return true;
	} else {
		return false;
	}
}

Config::Config() { }

Config::Config(std::string file) : _file(file) {
	_attr=std::map<std::string, std::string>();
	parse();	
}

bool Config::open(std::string file) {
	_file=file;
	return parse();
}

std::string &Config::operator[](const std::string &attr) {
	std::map<std::string, std::string>::iterator it = _attr.find(attr);
	if(it!=_attr.end()) {
		return it->second;
	} else {
		throw std::exception();
	}
}

