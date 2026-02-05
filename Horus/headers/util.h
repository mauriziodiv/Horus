#pragma once
#include "test.h"
#include <string>
#include <vector>

TestSelection StringToTestSelection(std::string_view testName);

std::vector<std::string> GetMainLineArgs(char* argv[]);