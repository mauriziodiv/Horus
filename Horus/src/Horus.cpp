#include "Horus.h"
#include "util.h"
#include "test.h"
#include "hrs.h"
#include "scene.h"
#include <iostream>
#include <thread>
#include <imfThreading.h>
//#include <ImfHeader.h>
//#include <OpenEXRConfig.h>

// Validates the input arguments for the program.
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

void setAOVs(Scene& scene, std::string s)
{
	scene.setDiffuseAOV(s);
	scene.setSpecularAOV(s);
	scene.setEmissionAOV(s);
	scene.setRefractionAOV(s);
	scene.setSubsurfaceScatteringAOV(s);
	scene.setNormalAOV(s);
	scene.setDirectLightAOV(s);
	scene.setDepthAOV(s);
	scene.setPositionAOV(s);
	scene.setIndirectAOV(s);
};

// The main function of the program.
int main(int argc, char* argv[])
{

	// Check for testing mode
	int32_t testResult = Testing(argc, argv);

	if (testResult == 0 || testResult == 1)
	{
		return testResult;
	}

	// Main program
	 
	std::cout << "Horus" << std::endl;

	unsigned int exrThreads = std::thread::hardware_concurrency();

	if (exrThreads == 0) { exrThreads = 4; }

	Imf::setGlobalThreadCount(static_cast<int>(exrThreads));

	std::vector<std::string> inputDescription;

	// Get command line arguments
	inputDescription = GetMainLineArgs(argv);

	// Validate input arguments
	if (!inputValidation(inputDescription)) { return 1; }

	std::vector<std::unique_ptr<SceneObject>> sceneObjects;
	bool result = SceneBuilder(inputDescription[0], sceneObjects);

	// Check if scene was built correctly
	if (!result) { return 1; }

	// Check if scene contains objects
	if (sceneObjects.size() == 0) { return 1; }

	Scene scene;

	// Set the destination of the render; its extension sets the render output type
	if (!scene.setFilePathWrite(inputDescription[1])) { return 1; }

	// Set gamma correction option if provided
	if (inputDescription.size() > 2 && !inputDescription[2].empty())
	{
		if (!scene.setGammaCorrection(inputDescription[2]))
		{
			std::cout << "Invalid Gamma Correction option!" << std::endl;
		}
	}

	//Set the Window
	if (inputDescription.size() > 3 && !inputDescription[3].empty())
	{
		if (!scene.setWindow(inputDescription[3]))
		{
			std::cout << "Invalid Window!" << std::endl;
		}
	}

	//Set the number of samples
	if (inputDescription.size() > 4 && !inputDescription[4].empty())
	{
		if (!scene.setSamples(inputDescription[4]))
		{
			std::cout << "Invalid Number of Samples!" << std::endl;
		}
	}

	//Set the number of bounces
	if (inputDescription.size() > 5 && !inputDescription[5].empty())
	{
		if (!scene.setBounces(inputDescription[5]))
		{
			std::cout << "Invalid Number of Bounces!" << std::endl;
		}
	}

	// diffuse AOV
	if (inputDescription.size() > 6 && !inputDescription[6].empty())
	{
		//scene.setDiffuseAOV(inputDescription[6]);
		setAOVs(scene, inputDescription[6]);
	}

	// specular AOV
	if (inputDescription.size() > 7 && !inputDescription[7].empty())
	{
		//scene.setSpecularAOV(inputDescription[7]);
		setAOVs(scene, inputDescription[7]);
	}

	// emission AOV
	if (inputDescription.size() > 8 && !inputDescription[8].empty())
	{
		//scene.setEmissionAOV(inputDescription[8]);
		setAOVs(scene, inputDescription[8]);
	}

	if (inputDescription.size() > 9 && !inputDescription[9].empty())
	{
		//scene.setRefractionAOV(inputDescription[9]);
		setAOVs(scene, inputDescription[9]);
	}

	if (inputDescription.size() > 10 && !inputDescription[10].empty())
	{
		//scene.setSubsurfaceScatteringAOV(inputDescription[10]);
		setAOVs(scene, inputDescription[10]);
	}

	if (inputDescription.size() > 11 && !inputDescription[11].empty())
	{
		//scene.setNormalAOV(inputDescription[11]);
		setAOVs(scene, inputDescription[11]);
	}

	if (inputDescription.size() > 12 && !inputDescription[12].empty())
	{
		setAOVs(scene, inputDescription[12]);
	}

	if (inputDescription.size() > 13 && !inputDescription[13].empty())
	{
		setAOVs(scene, inputDescription[13]);
	}

	if (inputDescription.size() > 14 && !inputDescription[14].empty())
	{
		setAOVs(scene, inputDescription[14]);
	}

	if (inputDescription.size() > 15 && !inputDescription[15].empty())
	{
		setAOVs(scene, inputDescription[15]);
	}

	// Get scene from scene objects
	if (!scene.getScene(sceneObjects)) { return 1; }

	// Render the scene
	scene.render();

	return 0;
}