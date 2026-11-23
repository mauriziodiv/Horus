#include "test.h"
#include "hrs.h"
#include "util.h"
#include <iostream>
#include <opensubdiv/far/topologyDescriptor.h>

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
int32_t Testing(int& argc, char* argv[])
{
	if (argc != 1 && std::string(argv[1]) == "--test")
	{

		std::cout << "syntax for testing:" << std::endl;
		std::cout << "Horus --test [TEST_NAME] [ARGUMENTS]" << std::endl;
		std::cout << std::endl;

		if (argc == 3 && std::string(argv[2]) == "--list")
		{
			// List available tests
			std::cout << "Available Tests:" << std::endl;
			std::cout << "  MAIN_LINE_ARGS" << std::endl;
			std::cout << "  SCENE_BUILDER" << std::endl;
			std::cout << "  SUBDIVISION" << std::endl;
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
		for (int32_t i = 3; i < argc; ++i)
		{
			testArgs.push_back(std::string(argv[i]));
		}

		RunTests(testName, testArgs);
		return 0;
	}

	return -1;
}

// Convert string to TestSelection enum
TestSelection StringToTestSelection(std::string_view testName)
{
	if (testName == "MAIN_LINE_ARGS") return TestSelection::MAIN_LINE_ARGS;
	if (testName == "SCENE_BUILDER") return TestSelection::SCENE_BUILDER;
	if (testName == "SUBDIVISION") return TestSelection::SUBDIVISION;

	return TestSelection::DEFAULT;
}

// Test: GetMainLineArgs
void T_MAIN_LINE_ARGS(const std::vector<std::string>& args)
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

	for (size_t i = 0; i < args.size(); ++i)
	{
		argv[i + 1] = const_cast<char*>(args[i].c_str());
	}
	argv[args.size() + 1] = nullptr;  // Null terminator

	// Call the actual function being tested
	std::vector<std::string> result = GetMainLineArgs(argv);

	// Display results
	std::cout << "Input has " << args.size() << " arguments" << std::endl;
	std::cout << "GetMainLineArgs returned " << result.size() << " elements:" << std::endl;

	for (size_t i = 0; i < result.size(); ++i)
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

// Test: SceneBuilder
void T_SCENE_BUILDER(const std::vector<std::string>& args)
{
	std::cout << "Scene Builder Test Running" << std::endl;

	if (args.empty())
	{
		std::cout << "Not enough arguments provided for test." << std::endl;
	}
	else
	{
		// Call the actual function being tested
		std::vector<std::unique_ptr<SceneObject>> sceneObjects;
		bool result = SceneBuilder(args[0], sceneObjects);
		
		// Display results
		if (result)
		{
			std::cout << "SceneBuilder successfully parsed the scene file." << std::endl;
			std::cout << "Number of scene objects created: " << sceneObjects.size() << std::endl;

			for (size_t i = 0; i < sceneObjects.size(); ++i)
			{
				std::cout << "Scene Object[" << i << "]:" << std::endl;
				sceneObjects[i]->printProperties();
				std::cout << std::endl;
			}
		}
		else
		{
			std::cout << "SceneBuilder failed to parse the scene file." << std::endl;
		}
	}
}

void T_SUBDIVISION(const std::vector<std::string>& args)
{
	std::cout << "Subdivision Test Running" << std::endl;

	// Cube topology: 6 quads, 8 vertices
	int vertsPerFace[6] = { 4, 4, 4, 4, 4, 4 };

	int vertIndices[24] = { 0, 1, 3, 2,
							2, 3, 5, 4,
							4, 5, 7, 6,
							6, 7, 1, 0,
							1, 7, 5, 3,
							6, 0, 2, 4 };

	OpenSubdiv::Far::TopologyDescriptor desc;
	desc.numVertices = 8;
	desc.numFaces = 6;
	desc.numVertsPerFace = vertsPerFace;
	desc.vertIndicesPerFace = vertIndices;

	// Build the refiner from the descriptor: Catmull-Clark scheme
	typedef OpenSubdiv::Far::TopologyRefinerFactory<OpenSubdiv::Far::TopologyDescriptor> RefinerFactory;

	OpenSubdiv::Far::TopologyRefiner* refiner = RefinerFactory::Create(desc,
		RefinerFactory::Options(OpenSubdiv::Sdc::SCHEME_CATMARK));

	if (refiner == nullptr)
	{
		std::cout << "[FAIL] OpenSubdiv could not build the refiner" << std::endl;
		return;
	}

	// Level 0 = the original cube, before any subdivision
		// Subdivide uniformly: every face, 2 levels
	refiner->RefineUniform(OpenSubdiv::Far::TopologyRefiner::UniformOptions(2));

	for (int level = 0; level <= refiner->GetMaxLevel(); ++level)
	{
		std::cout << "Level " << level << ": "
			<< refiner->GetLevel(level).GetNumVertices() << " vertices, "
			<< refiner->GetLevel(level).GetNumFaces() << " faces, "
			<< refiner->GetLevel(level).GetNumEdges() << " edges" << std::endl;
	}

	delete refiner;
}

// Run specified tests
void RunTests(TestSelection Test, const std::vector<std::string>& args)
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

	case TestSelection::SUBDIVISION:
		T_SUBDIVISION(args);
		break;

	default:
		break;

	}

	std::cout << "Tests Completed." << std::endl;
}