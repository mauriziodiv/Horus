#include "hrs.h"
#include <fstream>

// Tokenization function

bool SceneBuilder(std::string filePath, std::vector<SceneObject*>& sceneObjects)
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
			file.get();

			while (!file.eof() && file.peek() != ')')
			{
				token += file.get();
			}
			file.get();

			if (GeometryObjectsMap.find(token) != GeometryObjectsMap.end())
			{
				GeometryType gType = GeometryObjectsMap[token];

				switch (gType)
				{
				case GeometryType::SPHERE:
					// Create a sphere object
					sceneObjects.push_back(new SphereObject(1.0f));
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