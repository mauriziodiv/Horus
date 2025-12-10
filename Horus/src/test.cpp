#include "test.h"
#include "hrs.h"
#include "util.h"
#include <iostream>

// Helper function to convert SceneObjectType to string
const char* SceneObjectTypeToString(SceneObjectType type)
{
	switch (type)
	{
		case SceneObjectType::GEOMETRY: return "GEOMETRY";
		case SceneObjectType::LIGHT: return "LIGHT";
		case SceneObjectType::CAMERA: return "CAMERA";
		default: return "UNKNOWN";
	}
}

// Testing routine entry point
int Testing(int& argc, char* argv[])
{
	if (argc != 1 && std::string(argv[1]) == "--test")
	{

		std::cout << "syntax for testing:" << std::endl;
		std::cout << "Horus --test [TEST_NAME] [ARGUMENTS]" << std::endl;
		std::cout << std::endl;

		if (argc == 3 && std::string(argv[2]) == "--list")
		{
			// print a lise automatically
			std::cout << "Available Tests:" << std::endl;
			std::cout << "  MAIN_LINE_ARGS" << std::endl;
			std::cout << "  SCENE_BUILDER" << std::endl;
			return 1;
		 }

		if (argc < 3)
		{
			std::cout << "Please specify a test to run." << std::endl;
			return 1;
		}

		std::string testNameS = argv[2];
		TestSelection testName = StringToTestSelection(testNameS);

		std::vector<std::string> testArgs;
		for (int i = 3; i < argc; ++i)
		{
			testArgs.push_back(std::string(argv[i]));
		}

		RunTests(testName, testArgs);
		return 0;
	}

	return -1;
}

// Convert string to TestSelection enum
TestSelection StringToTestSelection(const std::string& testName)
{
	if (testName == "MAIN_LINE_ARGS") return TestSelection::MAIN_LINE_ARGS;
	if (testName == "SCENE_BUILDER") return TestSelection::SCENE_BUILDER;

	return TestSelection::DEFAULT;
}

// Test: GetMainLineArgs
void T_MAIN_LINE_ARGS(std::vector<std::string> args)
{
	std::cout << "Testing GetMainLineArgs(argv)" << std::endl;

	if (args.empty())
	{
		std::cout << "Not enough arguments provided for test." << std::endl;
		return;
	}

	// Create a char** array from the test args
	char** argv = new char* [args.size() + 2];
	argv[0] = const_cast<char*>("program_name");  // argv[0] is program name

	for (int i = 0; i < args.size(); ++i)
	{
		argv[i + 1] = const_cast<char*>(args[i].c_str());
	}
	argv[args.size() + 1] = nullptr;  // Null terminator

	// Call the actual function being tested
	std::vector<std::string> result = GetMainLineArgs(argv);

	// Display results
	std::cout << "Input has " << args.size() << " arguments" << std::endl;
	std::cout << "GetMainLineArgs returned " << result.size() << " elements:" << std::endl;

	for (int i = 0; i < result.size(); ++i)
	{
		std::cout << "  result[" << i << "] = " << result[i] << std::endl;
	}

	// Verify correctness
	if (result.size() == args.size())
	{
		std::cout << "[PASS] Correct number of elements" << std::endl;
	}
	else
	{
		std::cout << "[FAIL] Expected " << args.size() << " elements, got " << result.size() << std::endl;
	}

	// Clean up
	delete[] argv;
}

void T_SCENE_BUILDER(std::vector<std::string> args)
{
	std::cout << "Scene Builder Test Runnig" << std::endl;

	if (args.empty())
	{
		std::cout << "Not enough arguments provided for test." << std::endl;
	}
	else
	{
		// Call the actual function being tested
		std::vector<SceneObject*> sceneObjects;
		bool result = SceneBuilder(args[0], sceneObjects);
		
		// Display results
		if (result)
		{
			std::cout << "SceneBuilder successfully parsed the scene file." << std::endl;
			std::cout << "Number of scene objects created: " << sceneObjects.size() << std::endl;

			for (int i = 0; i < sceneObjects.size(); ++i)
			{
				std::string object_name = sceneObjects[i]->getObjectName();
				Vector3D<float> position = sceneObjects[i]->getPosition();
				float size = sceneObjects[i]->getSize();
				float intensity = sceneObjects[i]->getIntensity();
				Vector3D<float> lookAt = sceneObjects[i]->get_lookAt();

				std::cout << "  SceneObject[" << i << "] Name: " << object_name;
				std::cout << std::endl;

				std::cout << "  position: " << position.GetX() << " " << position.GetY() << " " << position.GetZ();
				std::cout << std::endl;

				std::cout << "  size: " << size;
				std::cout << std::endl;

				std::cout << "  intensity: " << intensity;
				std::cout << std::endl;

				std::cout << "  look at: " << lookAt.GetX() << " " << lookAt.GetY() << " " << lookAt.GetZ();
				std::cout << std::endl << std::endl;
			}
		}
		else
		{
			std::cout << "SceneBuilder failed to parse the scene file." << std::endl;
		}
	}
}	

// Run specified tests
void RunTests(TestSelection Test, std::vector<std::string> args)
{
	std::cout << "Running Tests: " << std::endl;

	switch (Test)
	{	

	case TestSelection::MAIN_LINE_ARGS:
		 T_MAIN_LINE_ARGS(args);
		 break;

    case TestSelection::SCENE_BUILDER:
		 T_SCENE_BUILDER(args);
		 break;

	default:
		break;

	}

	std::cout << "Tests Completed." << std::endl;
}