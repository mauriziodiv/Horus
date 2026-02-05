#include "util.h"
#include <fstream>

// Parses the command line arguments passed to the program and returns them as a vector of strings, excluding the program name.
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