#pragma once
#include <string>
#include <vector>

enum class TestSelection {
	DEFAULT,
	MAIN_LINE_ARGS,
	SCENE_BUILDER
};

int Testing(int& argc, char* argv[]);

TestSelection StringToTestSelection(const std::string&);

void RunTests(TestSelection Test, std::vector<std::string> args = {});