#include "hrs.h"
#include <fstream>
#include <sstream>

std::unordered_map<std::string, ParameterType> parameterMap = {
	{"pos", ParameterType::POSITION},
	{"rot", ParameterType::ROTATION},
	{"size", ParameterType::SIZE},
	{"radius", ParameterType::RADIUS},
	{"intensity", ParameterType::INTENSITY},
	{"lat", ParameterType::LAT},
	{"window", ParameterType::WINDOW},
	{"shader", ParameterType::SHADER}
};

std::unordered_map<std::string_view, ShaderType> shaderTypeMap = {
	{"constant", ShaderType::CONSTANT}
};

std::unordered_map<std::string_view, ShaderParameterType> shaderParameterMap = {
	{"color", ShaderParameterType::COLOR}
};

Ray CameraObject::genRay(float u, float v)
{
	Vector3D<float> direction = lower_left_corner + (horizontal * u) + (vertical * v) - position;
	direction.normalize();

	return Ray(position, direction);
}

// Reads characters from the file until it finds the specified character 'c', storing the characters in the 'token' string.
void charSearch(std::ifstream& file, char c, std::string& token)
{
	file.get();

	while (!file.eof() && file.peek() != c)
	{
		token += file.get();
	}

	file.get();
}

// Reads characters from the file, skipping whitespace, until it finds the specified character 'c', storing the characters in the 'token' string.
void tokenSearch(std::ifstream& file, char c, std::string& token)
{
	token.clear();

	while (!file.eof() && std::isspace(file.peek()))
	{
		file.get();
	}

	if (file.peek() == c) // What if this is different from '-'? It should keep searching for it ????????
	{
		file.get();

		while (!file.eof() && file.peek() != c)
		{
			token += file.get();
		}

		file.get();
		}
}

// Sets the parameters of the last created scene object based on the tokens read from the file.
void setObjectParameters(std::ifstream& file, std::string& token, std::vector<SceneObject*>& sceneObjects)
{
	while (!file.eof() && file.peek() != ';')
	{
		tokenSearch(file, '-', token);

		if (parameterMap.find(token) != parameterMap.end())
		{
			ParameterType pType = parameterMap[token];

			switch (pType)
			{
				case ParameterType::POSITION:

					tokenSearch(file, '/', token);

					if (!token.empty())
					{
						std::stringstream s(token);

						float x, y, z;
						char comma;

						s >> x >> comma >> y >> comma >> z;
						sceneObjects.back()->position = Vector3D<float>(x, y, z);
					}
					break;

					case ParameterType::ROTATION:

					tokenSearch(file, '/', token);

					if (!token.empty())
					{
						std::stringstream s(token);

						float x, y, z;
						char comma;

						s >> x >> comma >> y >> comma >> z;
						sceneObjects.back()->rotation = Vector3D<float>(x, y, z);
					}
					break;

				case ParameterType::SIZE:

					tokenSearch(file, '/', token);

					if (!token.empty())
					{
						GeometryObject* geometryObject = dynamic_cast<GeometryObject*>(sceneObjects.back());
						
						if (geometryObject)
						{
							geometryObject->size = std::stof(token);
						}
					}
					break;

				case ParameterType::INTENSITY:

					tokenSearch(file, '/', token);

					if (!token.empty())
					{
						LightObject* lightObject = dynamic_cast<LightObject*>(sceneObjects.back());
						
						if (lightObject)
						{
							lightObject->intensity = std::stof(token);
						}
					}
					break;

				case ParameterType::RADIUS:

						tokenSearch(file, '/', token);

						if (!token.empty())
						{
							SphereObject* sphereObject = dynamic_cast<SphereObject*>(sceneObjects.back());

							if (sphereObject)
							{
								sphereObject->size = std::stof(token);
							}
						}
						break;

				case ParameterType::LAT:

					tokenSearch(file, '/', token);

					if (!token.empty())
					{
						std::stringstream s(token);

						float x, y, z;
						char comma;

						s >> x >> comma >> y >> comma >> z;
						CameraObject* cameraObject = dynamic_cast<CameraObject*>(sceneObjects.back());

						if (cameraObject)
						{
							cameraObject->lookAt = Vector3D<float>(x, y, z);
						}
					}
					break;

				case ParameterType::WINDOW:

					tokenSearch(file, '/', token);

					if (!token.empty())
					{
						CameraObject* cameraObject = dynamic_cast<CameraObject*>(sceneObjects.back());

						if (cameraObject)
						{
							std::stringstream s(token);

							float width, height;
							char comma;

							s >> width >> comma >> height;

							cameraObject->setWindow(width, height);
						}
					}
					break;

				case ParameterType::SHADER:

					tokenSearch(file, '/', token);

					if (!token.empty())
					{
						GeometryObject* geometryObject = dynamic_cast<GeometryObject*>(sceneObjects.back());

						if (geometryObject)
						{
							geometryObject->linkShader(token);
						}
					}
					break;
				}
		}
		else if (!token.empty())
		{
			tokenSearch(file, '/', token);
		}
	}
}

// Builds the scene by reading the specified file and creating scene objects based on the tokens found in the file. The created objects are stored in the 'sceneObjects' vector.
bool SceneBuilder(const std::string& filePath, std::vector<SceneObject*>& sceneObjects)
{

	std::ifstream file(filePath);

	std::unordered_map<std::string, GeometryType> GeometryObjectsMap = 
	{
		{ "sphere", GeometryType::SPHERE }
	};

	std::unordered_map<std::string, LightType> LightObjectsMap =
	{
		{ "point", LightType::POINT }
	};

	std::unordered_map<std::string, CameraType> CameraObjectsMap =
	{
		{ "perspective", CameraType::PERSPECTIVE }
	};

	if (!file.is_open())
	{
		return false;
	}

	while (!file.eof())
	{
		std::string token;
		token.clear();

		if (file.peek() == '(')
		{
			charSearch(file, ')', token);

			if (GeometryObjectsMap.find(token) != GeometryObjectsMap.end())
			{
				GeometryType gType = GeometryObjectsMap[token];

				switch (gType)
				{
					case GeometryType::SPHERE:
						// Create a sphere object
						sceneObjects.push_back(new SphereObject(1.0f));
						setObjectParameters(file, token, sceneObjects);
						break;
				}
			}

			if (LightObjectsMap.find(token) != LightObjectsMap.end())
			{
				LightType lType = LightObjectsMap[token];

				switch (lType)
				{
					case LightType::POINT:
						// Create a point light object
						sceneObjects.push_back(new PointLightObject(1.0f));
						setObjectParameters(file, token, sceneObjects);
						break;
				}
			}

			if (CameraObjectsMap.find(token) != CameraObjectsMap.end())
			{
				CameraType cType = CameraObjectsMap[token];

				switch (cType)
				{
					case CameraType::PERSPECTIVE:
						// Create a perspective camera object
						sceneObjects.push_back(new PerspectiveCameraObject(45.0f));
						setObjectParameters(file, token, sceneObjects);
						break;
				}
			}
		}
		else
		{
			file.get();
		}
	}

	if (sceneObjects.empty())
	{
		return false;
	}

	return true;
}

// Links a shader to the geometry object by reading the shader file specified by 'shaderFilePath' and parsing its contents to set the shader properties.
bool GeometryObject::linkShader(std::string& shaderFilePath)
{
	shaderFile.open(shaderFilePath);

	if (!shaderFile.is_open())
	{
		std::cout << "Failed to open shader file: " << shaderFilePath << std::endl;
		return false;
	}

	parse();

	return true;
}

// Assigns a shader to the geometry object based on the specified shader type. Returns true if the shader was successfully assigned, false otherwise.
bool GeometryObject::assignShader(ShaderType sType)
{
	switch(sType)
	{
		case ShaderType::CONSTANT:
			shader = Constant();
			return true;

		default:
			break;
	}
	return false;
}

// Parses the shader file to extract shader properties and assign them to the geometry object. Returns true if parsing was successful, false otherwise.
bool GeometryObject::parse()
{
	token.clear();

	while (!shaderFile.eof() || shaderFile.peek() != ';')
	{
		if (shaderFile.peek() == '(')
		{
			charSearch(shaderFile, ')', token);

			if (shaderTypeMap.find(token) != shaderTypeMap.end())
			{
				ShaderType sType = shaderTypeMap[token];

				switch (sType)
				{
					case ShaderType::CONSTANT:
						if (assignShader(sType))
						{
							token.clear(); 

							tokenSearch(shaderFile, '-', token);

							ShaderParameterType spType;

							if (shaderParameterMap.find(token) != shaderParameterMap.end())
							{
								spType = shaderParameterMap[token];
								
								switch (spType)
								{
									case ShaderParameterType::COLOR:

										token.clear();

										tokenSearch(shaderFile, '/', token);

										if (!token.empty())
										{
											if (auto* p = std::get_if<Constant>(&getShader()))
											{
												// repeated code
												std::stringstream s(token);

												float x, y, z;
												char comma;

												s >> x >> comma >> y >> comma >> z;

												p->setColor(Vector3D<float>(x, y, z));
											}
										}

										break;

									default:
										return false;
								}

								
							}
						}
						return false;

					default:
						return false;
				}
			}
		}
	}
	return false;
}