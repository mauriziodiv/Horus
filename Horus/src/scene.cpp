#include "scene.h"

bool parseFloat(std::string_view token, float& v)
{
	const char* first = token.data();
	const char* last = first + token.size();

	std::from_chars_result res = std::from_chars(first, last, v);

	return res.ec == std::errc{} && res.ptr == last;
}

std::unordered_map<std::string_view, RenderOutput> renderOutputMap = {
	{ "ppm", RenderOutput::PPM }
};

std::unordered_map<std::string_view, GammaCorrection> gammaCorrectionMap = {
	{"gamma2", GammaCorrection::GAMMA2}
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
			GeometryObject* geometryObject = static_cast<GeometryObject*>(geo.get());

			if (geometryObject->getGeometryType() == GeometryType::MESH)
			{
				std::vector<GeometryObject*> triangles = geometryObject->getTriangles();
				geometries.insert(geometries.end(), triangles.begin(), triangles.end());
			}
			else
			{
				geometries.push_back(geometryObject);
			}
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
	areaLights.clear();
	meshLights.clear();

	for (const auto& lgts : sceneObjects)
	{
		if (lgts->getType() == SceneObjectType::LIGHT)
		{
			lights.push_back(static_cast<LightObject*>(lgts.get()));
		}
		else if (AreaLight* areaLight = dynamic_cast<AreaLight*>(lgts.get()))
		{
			areaLights.push_back(areaLight);
		}
		else if (MeshLight* meshLight = dynamic_cast<MeshLight*>(lgts.get()))
		{
			meshLight->buildCDF();
			meshLights.push_back(meshLight);
			//meshLights.buildCDF();
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

bool Scene::setGammaCorrection(const std::string_view gc)
{
	std::string_view par = (gc.front() == '-') ? gc.substr(1) : gc;

	if (gammaCorrectionMap.find(par) == gammaCorrectionMap.end())
	{
		return false;
	}

	gammaCorrection = gammaCorrectionMap[par];
	gammaCorrectionSet = true;

	return true;
}

bool Scene::setWindow(const std::string_view wSize)
{
	float w = 0.0f;
	float h = 0.0f;

	std::string_view token = (wSize.front() == '-') ? wSize.substr(1) : wSize;

	size_t sep = token.find(",");

	if (sep == std::string_view::npos) { return false; }

	std::string_view w_token = token.substr(0, sep);
	std::string_view h_token = token.substr(sep + 1);

	if (!parseFloat(w_token, w) || !parseFloat(h_token, h)) { return false; }

	width = w;
	height = h;

	return true;
}

bool Scene::setFocalLength(const std::string_view fl)
{
	float f = 0.0;
	std::string_view fl_token = (fl.front() == '-') ? fl.substr(1) : fl;

	if (!parseFloat(fl_token, f)) { return false; };

	focal_length = f;

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
	output.setWidth(getWidth());
	output.setHeight(getHeight());

	camera->setFocalLength(focal_length);
	camera->setWindow(getWidth(), getHeight());
	
	std::cout << "camera position : " << camera->getPosition().x << " " << camera->getPosition().y << " " << camera->getPosition().z << std::endl;

	std::cout << camera->getWidth() << " " << camera->getHeight() << std::endl;

	float width = camera->getWidth();
	float height = camera->getHeight();

	Integrator integrator(lights);
	integrator.addAreaLights(areaLights);
	integrator.addMeshLights(meshLights);

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

				color += integrator.rayPath(ray, bvh, 5);
			}

			color /= (float)numberOfSamples;

			if (gammaCorrectionSet)
			{
				switch (gammaCorrection)
				{
					case GammaCorrection::GAMMA2:
						color.x = std::sqrt(color.x);
						color.y = std::sqrt(color.y);
						color.z = std::sqrt(color.z);
						break;

					default:
						break;
				}
			}

			output.writeBuffer(color);
		}
	}
	output.write();
}