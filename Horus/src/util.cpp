#include "util.h"
#include <fstream>

std::vector<std::string> GetMainLineArgs(char* argv[])
{
	std::vector<std::string> result;

	++argv; 

	while (*argv)
	{
		result.push_back(std::string(*argv));
		++argv;
	}

	return result;
}