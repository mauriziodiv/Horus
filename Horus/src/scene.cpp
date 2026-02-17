#include "scene.h"

std::unordered_map<std::string_view, RenderOutput> renderOutputMap = {
	{ "ppm", RenderOutput::PPM }
};

Scene::Scene() : sceneObjects(), camera(nullptr), geometries(), lights(), renderOutput(RenderOutput::PPM), output(), filePathWrite()
{

}

// Initializes the scene by checking for the presence of a camera, geometries, and lights. Returns true if the scene is valid, false otherwise.
bool Scene::getScene(const std::vector<SceneObject*>& scene)
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

// Checks for the presence of a camera.
bool Scene::cameraCheck()
{
	for (SceneObject* obj : sceneObjects)
	{
		if (obj->getType() == SceneObjectType::CAMERA)
		{
			camera = static_cast<CameraObject*>(obj);
			camera->setPosition(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
			camera->setWindow(camera->getWidth(), camera->getHeight());
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

// Checks for the presence of geometries and populates the geometries vector.
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

// Checks for the presence of lights and populates the lights vector.
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

// Sets the render output format based on the provided string key.
bool Scene::setRenderOutput(const std::string_view& ro)
{
	std::string_view par = (ro.front() == '-') ? ro.substr(1) : ro;

	if (renderOutputMap.find(par) == renderOutputMap.end())
	{
		std::cout << "Invalid render output format!" << std::endl;
		return false;
	}

	renderOutput = renderOutputMap[par];

	return true;
}

// Sets the file path for writing the rendered output.
bool Scene::setFilePathWrite(const std::string_view& path)
{
	std::string_view par = (path.front() == '-') ? path.substr(1) : path;

	if (par.empty())
	{
		std::cout << "Invalid file path for writing!" << std::endl;
		return false;
	}

	filePathWrite = par;

	return true;
}

// Renders the scene.
void Scene::render()
{
	output.setRenderOutput(getRenderOutput());
	output.setFilePathWrite(getFilePathWrite());
	output.setWidth(getCamera()->getWidth());
	output.setHeight(getCamera()->getHeight());

	//camera->setPosition(getCamera()->getPosition());
	//camera->setWindow(getCamera()->getWidth(), getCamera()->getHeight());
	//std::cout << "camera position : " << camera->getPosition().x << " " << camera->getPosition().y << " " << camera->getPosition().z << std::endl;

	std::cout << camera->getWidth() << " " << camera->getHeight() << std::endl;

	float width = camera->getWidth();
	float height = camera->getHeight();

	for (int i = height - 1; i >= 0; --i)
	{
		for (int j = 0; j < width; ++j)
		{
			float u = (float)j / (width - 1);
			float v = (float)i / (height - 1);

			bool hit = false;

			Ray ray(camera->genRay(u, v));

			for (GeometryObject* geometry : geometries)
			{
				//std::cout << "HIT at u=" << u << " v=" << v << std::endl;
				if (geometry->getGeometryType() == GeometryType::SPHERE)
				{
					if (geometry->rayIntersection(ray, ray.getTMin(), ray.getTMax()))
					{
						output.writeBuffer(Vector3D(1.0f, 0.0f, 0.0f));
						hit =true;
					}
				}
			}
			if (!hit)
			{
				output.writeBuffer(Vector3D(0.0f, 0.0f, 0.0f));
			}
		}
	}
	output.write();
}