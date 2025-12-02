#pragma once
#include <string>
#include <vector>
#include <unordered_map>

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

		SceneObject(SceneObjectType t)
		{
			type = t;
		}

		SceneObjectType getType()
		{
			return type;
		}

		void setPosition(int p)
		{
			position = p;
		}

		int getPosition()
		{
			return position;
		}
		
		virtual std::string getObjectName() = 0;

	private:
		SceneObjectType type;

		int position;

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
		CameraObject(CameraType cType) : SceneObject(SceneObjectType::CAMERA), cameraType(cType) {}

		CameraType getCameraType()
		{
			return cameraType;
		}
		
	private:
		CameraType cameraType;
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