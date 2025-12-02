#pragma once
#include "test.h"
#include <string>
#include <vector>

TestSelection StringToTestSelection(const std::string& testName);

std::vector<std::string> GetMainLineArgs(char* argv[]);