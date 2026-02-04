#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "vec_math.h"
#include "ray.h"

enum class ParameterType {
	POSITION,
	SIZE,
	RADIUS,
	INTENSITY,
	LAT
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

		virtual void printProperties()
		{
			std::cout << "position: " << getPosition().getX() << " " << getPosition().getY() << " " << getPosition().getZ() << std::endl;
		}

		virtual void setIntensity(float i) {};
		virtual float getIntensity() { return 0.0f; };
		
		virtual std::string getObjectName() = 0;

	private:
		SceneObjectType type;
		Vector3D<float> position;

};

// GEOMETRY ###############################################

// Derived classes for specific scene objects
class GeometryObject : public SceneObject {

	public:

		GeometryObject(GeometryType gType) : SceneObject(SceneObjectType::GEOMETRY), geometryType(gType) {}

		GeometryType getGeometryType()
		{
			return geometryType;
		}

		virtual void setSize (float s)
		{
			size = s;
		}

		virtual float getSize ()
		{
			return size;
		}

		virtual void printProperties() override
		{
			SceneObject::printProperties();
			//std::cout << "size: " << size << std::endl;
		}

		virtual float rayIntersection(Ray& ray, float tMin, float tMax, float t) { return -1.0f; };

	private:
		GeometryType geometryType;
		float size;
};

class SphereObject : public GeometryObject {
	public:
		SphereObject(float r = 1.0f) : GeometryObject(GeometryType::SPHERE), radius(r) { GeometryObject::setSize(r); }
		
		std::string getObjectName() override
		{
			return name;
		}

		void setSize(float r) override
		{
			GeometryObject::setSize(r);
			radius = r;
		}

		float getSize() override
		{
			return radius;
		}

		virtual void printProperties() override
		{
			GeometryObject::printProperties();
			std::cout << "Type: " << getObjectName() << std::endl;
			std::cout << "radius: " << radius << std::endl;
		}

		float rayIntersection(Ray& ray, float tMin, float tMax, float t) override
		{
			float a = ray.getDirection() * ray.getDirection();

			Vector3D<float> oc = ray.getOrigin() - getPosition();
			float b = ((ray.getDirection()) * oc) * 2.0f;

			float c = (oc * oc) - (radius * radius);

			float discriminant = (b * b) - (4 * a * c);

			if (discriminant < 0)
			{
				return -1.0f;
			}

			float sqr = std::sqrt(discriminant);

			float t1 = (-b - sqr) / (2.0f * a);
			float t2 = (-b + sqr) / (2.0f * a);

			if (t1 > tMin && t1 < tMax)
			{
				return t1;
			}

			if (t2 > tMin && t2 < tMax)
			{
				return t2;
			}
		
			return -1.0f;
		}

	private:
		std::string name = "Sphere";
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

		void setSize(float s)
		{
			size = s;
		}

		float getSize()
		{
			return size;
		}

		void setIntensity(float i) override
		{
			intensity = i;
		}

		float getIntensity() override
		{
			return intensity;
		}

		virtual void printProperties() override
		{
			SceneObject::printProperties();
			std::cout << "size: " << size << std::endl;
			std::cout << "intensity: " << intensity << std::endl;
		}

	private:
		LightType lightType;
		float size;
		float intensity;
};

class PointLightObject : public LightObject {
	public:
		PointLightObject(float i) : LightObject(LightType::POINT) { LightObject::setSize(1.0f); }

		std::string getObjectName() override
		{
			return name;
		}

		virtual void printProperties() override
		{
			LightObject::printProperties();
			std::cout << "Type: " << getObjectName() << std::endl;
		}

	private:
		std::string name = "Point Light";
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

		virtual void printProperties() override
		{
			SceneObject::printProperties();
			std::cout << "look at: " << get_lookAt().getX() << " " << get_lookAt().getY() << " " << get_lookAt().getZ() << std::endl;
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
			return name;
		}

		virtual void printProperties() override
		{
			CameraObject::printProperties();
			std::cout << "Type: " << getObjectName() << std::endl;
		}

	private:
		std::string name = "Camera Perspective";
		float fieldOfView;
};

bool SceneBuilder(std::string, std::vector<SceneObject*>&);

void tokenSearch(std::ifstream& file, char c, std::string& token);
void setObjectParameters(std::ifstream&, std::string&, std::vector<SceneObject*>&);