#include "hrs.h"
#include <fstream>
#include <sstream>

std::unordered_map<std::string, ParameterType> parameterMap = {
	{"pos", ParameterType::POSITION},
	{"size", ParameterType::SIZE}
};

void charSearch(std::ifstream& file, char c, std::string& token)
{
	file.get();

	while (!file.eof() && file.peek() != c)
	{
		token += file.get();
	}

	file.get();
}

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

void setObjectPosition(std::ifstream& file, std::string& token, std::vector<SceneObject*>& sceneObjects)
{
	tokenSearch(file, '-', token);

	if (parameterMap.find(token) != parameterMap.end())
	{
		ParameterType pType = parameterMap[token];

		if (pType == ParameterType::POSITION)
		{
			// Parse position values
			tokenSearch(file, '/', token);

			if (!token.empty())
			{
				std::stringstream s(token);

				float x, y, z;
				char comma;

				s >> x >> comma >> y >> comma >> z;
				sceneObjects.back()->setPosition(x, y, z);
			}
		}
	}
}

void setObjectParameters(std::ifstream& file, std::string& token)
{
	while (!file.eof() || file.peek() != ';')
	{
		tokenSearch(file, '-', token);

		if (!token.empty())
		{
			// FROM HERE
		}
	}
}

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
			charSearch(file, ')', token);

			if (GeometryObjectsMap.find(token) != GeometryObjectsMap.end())
			{
				GeometryType gType = GeometryObjectsMap[token];

				switch (gType)
				{
					case GeometryType::SPHERE:
						// Create a sphere object
						sceneObjects.push_back(new SphereObject(1.0f));
						setObjectPosition(file, token, sceneObjects);
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
						setObjectPosition(file, token, sceneObjects);
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
						setObjectPosition(file, token, sceneObjects);
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