#include "scene.h"
#include <thread>
#include <atomic>

bool parseFloat(std::string_view token, float& v)
{
	const char* first = token.data();
	const char* last = first + token.size();

	std::from_chars_result res = std::from_chars(first, last, v);

	return res.ec == std::errc{} && res.ptr == last;
}

std::unordered_map<std::string_view, RenderOutput> renderOutputMap = {
	{ "ppm", RenderOutput::PPM },
	{ "exr", RenderOutput::EXR }
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

bool Scene::setGammaCorrection(const std::string_view gc)
{
	std::string_view par = (gc.front() == '-') ? gc.substr(1) : gc;

	if (gammaCorrectionMap.find(par) == gammaCorrectionMap.end() || !gammaCorrectionEnabled)
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

bool Scene::setSamples(const std::string_view s)
{
	float samples = 0.0f;
	std::string_view samples_token = (s.front() == '-') ? s.substr(1) : s;

	if (!parseFloat(samples_token, samples)) { return false; };

	if (samples < 1.0f) { return false; }

	numberOfSamples = static_cast<int>(samples);

	return true;
}

bool Scene::setBounces(const std::string_view b)
{
	float bncs = 0.0f;
	std::string_view bounces_token = (b.front() == '-') ? b.substr(1) : b;

	if (!parseFloat(bounces_token, bncs)) { return false; };

	bounces = static_cast<int>(bncs);

	return true;
}

bool Scene::setDiffuseAOV(const std::string_view d)
{
	if (d.empty())
	{
		return false;
	}

	std::string_view diffuse_token = (d.front() == '-') ? d.substr(1) : d;

	if (diffuse_token == "d")
	{
		std::cout << "Diffuse AOV is enabled!" << std::endl;
		diffuseAOVEnabled = true;
	}

	return false;
}

bool Scene::setSpecularAOV(const std::string_view s)
{
	if (s.empty())
	{
		return false;
	}

	std::string_view specular_token = (s.front() == '-') ? s.substr(1) : s;

	if (specular_token == "s")
	{
		std::cout << "Specular AOV is enabled!" << std::endl;
		specularAOVEnabled = true;
	}

	return false;
}

bool Scene::setEmissionAOV(const std::string_view e)
{
	if (e.empty())
	{
		return false;
	}

	std::string_view emission_token = (e.front() == '-') ? e.substr(1) : e;

	if (emission_token == "e")
	{
		std::cout << "Emission AOV is enabled!" << std::endl;
		emissionAOVEnabled = true;
	}

	return false;
}

bool Scene::setRefractionAOV(const std::string_view r)
{
	if (r.empty())
	{
		return false;
	}

	std::string_view refraction_token = (r.front() == '-') ? r.substr(1) : r;

	if (refraction_token == "r")
	{
		std::cout << "Refraction AOV is enabled!" << std::endl;
		refractionAOVEnabled = true;
	}

	return false;
}

bool Scene::setSubsurfaceScatteringAOV(const std::string_view sb)
{
	if (sb.empty())
	{
		return false;
	}

	std::string_view sss_token = (sb.front() == '-') ? sb.substr(1) : sb;

	if (sss_token == "sb")
	{
		std::cout << "Subsurface Scattering AOV is enabled!" << std::endl;
		subsurfaceScatteringAOVEnabled = true;
	}

	return false;
}

bool Scene::setNormalAOV(const std::string_view n)
{
	if (n.empty())
	{
		return false;
	}

	std::string_view normal_token = (n.front() == '-') ? n.substr(1) : n;

	if (normal_token == "n")
	{
		std::cout << "Normal AOV is enabled!" << std::endl;
		normalAOVEnabled = true;
	}

	return false;
}

bool Scene::setDirectLightAOV(const std::string_view d)
{
	if (d.empty())
	{
		return false;
	}

	std::string_view directLight_token = (d.front() == '-') ? d.substr(1) : d;

	if (directLight_token == "dl")
	{
		std::cout << "Direct Light AOV is enabled!" << std::endl;
		directLightAOVEnabled = true;
	}
	return false;
}

bool Scene::setDepthAOV(const std::string_view d)
{
	if (d.empty())
	{
		return false;
	}

	std::string_view depth_token = (d.front() == '-') ? d.substr(1) : d;

	if (depth_token == "dp")
	{
		std::cout << "Depth AOV is enabled!" << std::endl;
		depthAOVEnabled = true;
	}
	return false;
}

bool Scene::setPositionAOV(const std::string_view p)
{
	if (p.empty())
	{
		return false;
	}

	std::string_view position_token = (p.front() == '-') ? p.substr(1) : p;

	if (position_token == "pos")
	{
		std::cout << "Position AOV is enabled!" << std::endl;
		positionAOVEnabled = true;
	}
	
	return false;
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

	size_t dot = par.rfind('.');

	if (dot == std::string_view::npos)
	{
		std::cout << "No extension found in file path!" << std::endl;
		return false;
	}

	std::string_view extension = par.substr(dot + 1);

	auto it = renderOutputMap.find(extension);

	if (it == renderOutputMap.end())
	{
		std::cout << "Unsupported file extension!" << std::endl;
		return false;
	}

	renderOutput = it->second;

	if (renderOutput == RenderOutput::EXR)
	{
		gammaCorrectionEnabled = false;
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
	output.allocateBuffer();

	if (diffuseAOVEnabled)
	{
		diffuseLayer = output.addLayer("diffuse");
	}

	if (specularAOVEnabled)
	{
		specularLayer = output.addLayer("specular");
	}

	if (emissionAOVEnabled)
	{
		emissionLayer = output.addLayer("emission");
	}

	if (refractionAOVEnabled)
	{
		refractionLayer = output.addLayer("refraction");
	}

	if (subsurfaceScatteringAOVEnabled)
	{
		subsurfaceScatteringLayer = output.addLayer("subsurface_scattering");
	}

	if (normalAOVEnabled)
	{
		normalLayer = output.addLayer("normal");
	}

	if (directLightAOVEnabled)
	{
		directLightLayer = output.addLayer("direct_light");
	}

	if (depthAOVEnabled)
	{
		depthLayer = output.addLayer("Z");
	}

	if (positionAOVEnabled)
	{
		positionLayer = output.addLayer("P");
	}

	camera->setWindow(getWidth(), getHeight());
	
	std::cout << "camera position : " << camera->getPosition().x << " " << camera->getPosition().y << " " << camera->getPosition().z << std::endl;

	std::cout << camera->getWidth() << " " << camera->getHeight() << std::endl;

	float width = camera->getWidth();
	float height = camera->getHeight();

	BVH bvh;
	bvh.buildBVH(geometries);

	auto renderRow = [&](int i, Integrator& integrator)
		{
			for (int j = 0; j < width; ++j)
			{
				integrator.seed(static_cast<uint32_t>(i * (int)width + j));

				float u = (float)j / (width - 1);
				float v = (float)i / (height - 1);

				Vector3D<float> color(0.0f, 0.0f, 0.0f);
				Vector3D<float> diffuseAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> specularAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> emissionAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> refractionAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> subsurfaceScatteringAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> normalAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> directLightAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> depthAOV(1.0e30f, 0.0f, 0.0f);
				Vector3D<float> positionAOV(1.0e30f, 1.0e30f, 1.0e30f);
				float alpha = 0.0f;

				Ray ray(camera->genRay(u, v));

				Ray originalRay = ray;

				for (size_t k = 0; k < numberOfSamples; ++k)
				{
					float x = (originalRay.getDirection().x + (integrator.getUnitRandom().Generate() - 0.5f) / width);
					float y = (originalRay.getDirection().y + (integrator.getUnitRandom().Generate() - 0.5f) / height);
					float z = (originalRay.getDirection().z + (integrator.getUnitRandom().Generate() - 0.5f) / width);

					ray.setDirection(Vector3D<float>(x, y, z));

					color += integrator.rayPath(ray, bvh, bounces, true, true, 0.0f, &diffuseAOV, &specularAOV, &emissionAOV, &refractionAOV, &subsurfaceScatteringAOV, &alpha, &normalAOV, &directLightAOV, &depthAOV, &positionAOV);
				}

				color /= (float)numberOfSamples;
				diffuseAOV /= (float)numberOfSamples;
				specularAOV /= (float)numberOfSamples;
				emissionAOV /= (float)numberOfSamples;
				refractionAOV /= (float)numberOfSamples;
				subsurfaceScatteringAOV /= (float)numberOfSamples;
				normalAOV /= (float)numberOfSamples;
				directLightAOV /= (float)numberOfSamples;
				alpha /= (float)numberOfSamples;

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

				output.writeBuffer(static_cast<size_t>((height - 1 - i) * width + j), color);
				output.writeAlpha(static_cast<size_t>((height - 1 - i) * width + j), alpha);

				if (diffuseAOVEnabled)
				{
					output.writeLayer(diffuseLayer, static_cast<size_t>((height - 1 - i) * width + j), diffuseAOV);
				}
				
				if (specularAOVEnabled)
				{
					output.writeLayer(specularLayer, static_cast<size_t>((height - 1 - i) * width + j), specularAOV);
				}

				if (emissionAOVEnabled)
				{
					output.writeLayer(emissionLayer, static_cast<size_t>((height - 1 - i) * width + j), emissionAOV);
				}

				if (refractionAOVEnabled)
				{
					output.writeLayer(refractionLayer, static_cast<size_t>((height - 1 - i) * width + j), refractionAOV);
				}

				if (subsurfaceScatteringAOVEnabled)
				{
					output.writeLayer(subsurfaceScatteringLayer, static_cast<size_t>((height - 1 - i) * width + j), subsurfaceScatteringAOV);
				}

				if (normalAOVEnabled)
				{
					output.writeLayer(normalLayer, static_cast<size_t>((height - 1 - i) * width + j), normalAOV);
				}

				if (directLightAOVEnabled)
				{
					output.writeLayer(directLightLayer, static_cast<size_t>((height - 1 - i) * width + j), directLightAOV);
				}

				if (depthAOVEnabled)
				{
					output.writeLayer(depthLayer, static_cast<size_t>((height - 1 - i) * width + j), depthAOV);
				}

				if (positionAOVEnabled)
				{
					output.writeLayer(positionLayer, static_cast<size_t>((height - 1 - i) * width + j), positionAOV);
				}
			}
		};

	unsigned int threadCount = std::thread::hardware_concurrency();

	if (threadCount == 0)
	{
		threadCount = 4;
	}

	std::atomic<int> nextRow{ 0 };

	std::vector<std::thread> threads;

	for (unsigned int t = 0; t < threadCount; ++t)
	{
		threads.emplace_back([&]()
			{
				Integrator integrator(lights);
				integrator.addAreaLights(areaLights);
				integrator.addMeshLights(meshLights);
				integrator.setCameraForward(camera->getForward());

				int i;

				while ((i = nextRow++) < (int)height)
				{
					renderRow(i, integrator);
				}
			});
	}

	for (std::thread& t : threads)
	{
		t.join();
	}

	output.write();
}