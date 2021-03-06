#ifndef DEV_LAYOUT_H
#define DEV_LAYOUT_H

#include <exception>
#include <iostream>
#include <fstream>

#include "json.h"
#include "debug.h"

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

struct DevLayout {
	nlohmann::json jconf;
	std::string confpath;

	DevLayout(std::string confpath) : confpath(confpath) {
		std::ifstream file(confpath.c_str());
		if (!file.good()) {
			EXCEPTION("Couldn't open file: %s\n", confpath.c_str());
			return ;
		}
		try {
			file >> jconf;
		}
		catch (std::exception& ex) {
			std::cout << ex.what() << std::endl;
		}
	}

	auto operator [] (std::string opt_name) {
		if (jconf.find(opt_name) == jconf.end())
			EXCEPTION("%s not found", opt_name.c_str());
		return jconf[opt_name];
	}

	auto path(std::string opt_name) {
		if (jconf.find(opt_name) == jconf.end())
			EXCEPTION("Path not found: %s", opt_name.c_str());
		fs::path p1 = confpath;
		return p1.parent_path().string() + "/" +
				jconf[opt_name].get<std::string>();
	}
};

#endif