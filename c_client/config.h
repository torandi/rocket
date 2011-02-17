#ifndef CONFIG_H_
#define CONFIG_H_

#include <map>
#include <string>

class Config {
	std::map<std::string, std::string> _attr;
	std::string _file;

	void parse();
	public:
		Config(std::string file);

		std::string &operator[](const std::string &attr);

		
};

#endif
