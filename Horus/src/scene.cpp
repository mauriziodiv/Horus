#include "scene.h"

std::unordered_map<std::string_view, RenderOutput> renderOutputMap = {
	{ "ppm", RenderOutput::PPM }
};

Scene::Scene() : sceneObjects(), camera(nullptr), geometries(), lights(), renderOutput(RenderOutput::PPM), output(), filePathWrite()
{

}

// Initializes the scene by checking for the presence of a camera, geometries, and lights. Returns true if the scene is valid, false otherwise.
bool Scene::getScene(std::vector<std::unique_ptr<SceneObject>>& scene)
{
	if (scene.empty())
	{
		return false;
	}

	//sceneObjects = scene;
	sceneObjects = std::move(scene);

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
	for (const auto& obj : sceneObjects)
	{
		if (obj->getType() == SceneObjectType::CAMERA)
		{
			camera = static_cast<CameraObject*>(obj.get());
			//camera->setPosition(camera->getPosition().x, camera->getPosition().y, camera->getPosition().z);
			//camera->setWindow(camera->getWidth(), camera->getHeight());
			//std::cout << "camera position : " << camera->getPosition().x << " " << camera->getPosition().y << " " << camera->getPosition().z << std::endl;
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

	for (const auto& geo : sceneObjects)
	{
		if (geo->getType() == SceneObjectType::GEOMETRY)
		{
			geometries.push_back(static_cast<GeometryObject*>(geo.get()));
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

	for (const auto& lgts : sceneObjects)
	{
		if (lgts->getType() == SceneObjectType::LIGHT)
		{
			lights.push_back(static_cast<LightObject*>(lgts.get()));
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

	camera->setWindow(camera->getWidth(), camera->getHeight());
	std::cout << "camera position : " << camera->getPosition().x << " " << camera->getPosition().y << " " << camera->getPosition().z << std::endl;

	std::cout << camera->getWidth() << " " << camera->getHeight() << std::endl;

	float width = camera->getWidth();
	float height = camera->getHeight();

	Integrator integrator(lights);

	BVH bvh;
	bvh.buildBVH(geometries);

	for (int i = height - 1; i >= 0; --i)
	{
		for (int j = 0; j < width; ++j)
		{
			float u = (float)j / (width - 1);
			float v = (float)i / (height - 1);

			Vector3D<float> color(0.0f, 0.0f, 0.0f);

			Ray ray(camera->genRay(u, v));

			Ray originalRay = ray;

			for (size_t k = 0; k < numberOfSamples; ++k)
			{
				float x = (originalRay.getDirection().x + (unitRandom.Generate() - 0.5f) / width);
				float y = (originalRay.getDirection().y + (unitRandom.Generate() - 0.5f) / height);
				float z = (originalRay.getDirection().z + (unitRandom.Generate() - 0.5f) / width);

				ray.setDirection(Vector3D<float>(x, y, z));

				color += integrator.rayPath(ray, bvh, 0);
			}

			color /= (float)numberOfSamples;

			output.writeBuffer(color);
		}
	}
	output.write();
}