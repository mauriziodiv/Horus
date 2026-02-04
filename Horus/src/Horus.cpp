#include "Horus.h"
#include "util.h"
#include "test.h"
#include "hrs.h"
#include "scene.h"
#include <iostream>

bool inputValidation(const std::vector<std::string>& inputDescription)
{
	switch (inputDescription.size())
	{
		case 0:
			std::cout << "no arguments defined!" << std::endl;
			return false;

		case 1:
			std::cout << "no render output defined!" << std::endl;
			return false;

		default:
			break;
	}

	return true;
}

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

	// Validate input arguments
	if (!inputValidation(inputDescription)) { return 1; }

	std::vector<SceneObject*> sceneObjects;
	bool result = SceneBuilder(inputDescription[0], sceneObjects);

	// Check if scene was built correctly
	if (!result) { return 1; }

	// Check if scene contains objects
	if (sceneObjects.size() == 0) { return 1; }

	Scene scene;

	// Set render output type
	scene.setRenderOutput(inputDescription[1]);

	// Get scene from scene objects
	if (!scene.getScene(sceneObjects)) { return 1; }

	// Render the scene
	scene.render();

	return 0;

}