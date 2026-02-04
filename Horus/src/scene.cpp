#include "scene.h"

std::unordered_map<std::string, RenderOutput> renderOutputMap = {
	{ "ppm", RenderOutput::PPM }
};

Scene::Scene() : sceneObjects(), camera(nullptr), geometries(), lights()
{

}

bool Scene::getScene(std::vector<SceneObject*> scene)
{
	if (scene.empty())
	{
		return false;
	}

	sceneObjects = scene;

	if (!cameraCheck())
	{
		return false;
	}

	geometriesCheck();
	lightCheck();

	return true;
}

bool Scene::cameraCheck()
{
	for (SceneObject* obj : sceneObjects)
	{
		if (obj->getType() == SceneObjectType::CAMERA)
		{
			camera = static_cast<CameraObject*>(obj);
			break;
		}
	}

	if (camera == nullptr)
	{
		std::cout << "There is no camera in the scene!" << std::endl;
		return false;
	}

	return true;
}

bool Scene::geometriesCheck()
{
	geometries.clear();

	for (SceneObject* geo : sceneObjects)
	{
		if (geo->getType() == SceneObjectType::GEOMETRY)
		{
			geometries.push_back(static_cast<GeometryObject*>(geo));
		}
	}

	if (geometries.size() == 0)
	{
		std::cout << "Warning: There are no geometries in the scene!" << std::endl;
	}

	return true;
}

bool Scene::lightCheck()
{
	lights.clear();

	for (SceneObject* lgts : sceneObjects)
	{
		if (lgts->getType() == SceneObjectType::LIGHT)
		{
			lights.push_back(static_cast<LightObject*>(lgts));
		}
	}

	if (lights.size() == 0)
	{
		std::cout << "Warning: There are no lights in the scene!" << std::endl;
	}

	return true;
}

void Scene::setRenderOutput(std::string ro)
{
	renderOutput = renderOutputMap[ro];
}

void Scene::render()
{

}