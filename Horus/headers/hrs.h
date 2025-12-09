#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "vec_math.h"

enum class ParameterType {
	POSITION,
	SIZE
};

extern std::unordered_map<std::string, ParameterType> parameterMap;

enum class SceneObjectType {
	GEOMETRY,
	LIGHT,
	CAMERA
};

enum class GeometryType {
	SPHERE
};

enum class LightType {
	POINT
};

enum class CameraType {
	PERSPECTIVE
};

// SCENE OBJECT BASE CLASS
class SceneObject {

	public:

		SceneObject(SceneObjectType t) : type(t), position(0.0f, 0.0f, 0.0f) {}

		SceneObjectType getType()
		{
			return type;
		}

		void setPosition(float x, float y, float z)
		{
			position = Vector3D<float>(x, y, z);
		}

		Vector3D<float> getPosition()
		{
			return position;
		}
		
		virtual std::string getObjectName() = 0;

	private:
		SceneObjectType type;

		Vector3D<float> position;

};

// GEOMETRY ###############################################

// Derived classes for specific scene objects
class GeometryObject : public SceneObject {
	public:
		GeometryObject(GeometryType gType) : SceneObject(SceneObjectType::GEOMETRY),  geometryType(gType) {}

		GeometryType getGeometryType()
		{
			return geometryType;
		}

	private:
		GeometryType geometryType;
};

class SphereObject : public GeometryObject {
	public:
		SphereObject(int r) : GeometryObject(GeometryType::SPHERE), radius(r) {}
		
		std::string getObjectName() override
		{
			return "Sphere";
		}

	private:
		float radius;
};

// LIGHT ################################################
// 
// Derived classes for specific scene objects
class LightObject : public SceneObject {
	public:
		LightObject(LightType lType) : SceneObject(SceneObjectType::LIGHT), lightType(lType) {}

		LightType getLightType()
		{
			return lightType;
		}

	private:
		LightType lightType;
};

class PointLightObject : public LightObject {
	public:
		PointLightObject(float i) : LightObject(LightType::POINT), intensity(i) {}

		std::string getObjectName() override
		{
			return "Point Light";
		}

	private:
		float intensity;
};

// CAMERA ###############################################

// Derived classes for specific scene objects
class CameraObject : public SceneObject {
	public:
		CameraObject(CameraType cType) : SceneObject(SceneObjectType::CAMERA), cameraType(cType), lookAt(Vector3D<float> (0.0f, 0.0f, 0.0f)) {}

		CameraType getCameraType()
		{
			return cameraType;
		}

		void set_lookAt(float x, float y, float z)
		{
			lookAt = Vector3D<float>(x, y, z);
		}

		Vector3D<float> get_lookAt()
		{
			return lookAt;
		}
		
	private:
		CameraType cameraType;
		Vector3D<float> lookAt;
};

class PerspectiveCameraObject : public CameraObject {
	public:
		PerspectiveCameraObject(float fov) : CameraObject(CameraType::PERSPECTIVE), fieldOfView(fov) {}

		std::string getObjectName() override
		{
			return "Camera Perspective";
		}

	private:
		float fieldOfView;
};

bool SceneBuilder(std::string, std::vector<SceneObject*>&);

void tokenSearch(std::ifstream& file, char c, std::string& token);
void setObjectPosition(std::ifstream&, std::string&, std::vector<SceneObject*>&);
void setObjectParamters(std::ifstream&, std::string&);