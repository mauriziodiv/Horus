#pragma once
#include <string>
#include <vector>
#include <string_view>

enum class TestSelection {
	DEFAULT,
	MAIN_LINE_ARGS,
	SCENE_BUILDER
};

int32_t Testing(int& argc, char* argv[]);

TestSelection StringToTestSelection(std::string_view testName);

void RunTests(TestSelection Test, const std::vector<std::string>& args = {});