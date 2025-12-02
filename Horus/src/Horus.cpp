#include "Horus.h"
#include "util.h"
#include "test.h"
#include "hrs.h"
#include <string>
#include <vector>
#include <iostream>

int main(int argc, char* argv[])
{

	// Check for testing mode
	int testResult = Testing(argc, argv);

	if (testResult == 0 || testResult == 1)
	{
		return testResult;
	}

	// Main program
	 
	std::cout << "Horus" << std::endl;

	std::vector<std::string> inputDescription;

	// Get command line arguments
	inputDescription = GetMainLineArgs(argv);

	// Extract tokens from hrs file

	return 0;

}