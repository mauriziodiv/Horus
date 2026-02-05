#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <string_view>
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

		//void setPosition(float x, float y, float z)
		//{
		//	position = Vector3D<float>(x, y, z);
		//}

		//Vector3D<float> getPosition()
		//{
		//	return position;
		//}

		virtual void printProperties()
		{
			std::cout << "position: " << position.x << " " << position.y << " " << position.z << std::endl;
		}

		virtual float getIntensity() { return 0.0f; };
		
		virtual std::string_view getObjectName() = 0;

		Vector3D<float> position;

	private:
		SceneObjectType type;
		

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

		//virtual void setSize (float s)
		//{
		//	size = s;
		//}

		//virtual float getSize ()
		//{
		//	return size;
		//}

		virtual void printProperties() override
		{
			SceneObject::printProperties();
			//std::cout << "size: " << size << std::endl;
		}

		virtual float rayIntersection(Ray& ray, float tMin, float tMax, float t) { return -1.0f; };

		float size;

	private:
		GeometryType geometryType;
		
};

class SphereObject : public GeometryObject {
	public:
		SphereObject(float r = 1.0f) : GeometryObject(GeometryType::SPHERE) { GeometryObject::size = r; }
		
		std::string_view getObjectName() override
		{
			return name;
		}

		//void setSize(float r) override
		//{
		//	GeometryObject::setSize(r);
		//	radius = r;
		//}

		//float getSize() override
		//{
		//	return radius;
		//}

		virtual void printProperties() override
		{
			GeometryObject::printProperties();
			std::cout << "Type: " << getObjectName() << std::endl;
			std::cout << "radius: " << size << std::endl;
		}

		float rayIntersection(Ray& ray, float tMin, float tMax, float t) override
		{
			float a = ray.direction * ray.direction;

			Vector3D<float> oc = ray.origin - position;
			float b = ((ray.direction) * oc) * 2.0f;

			float c = (oc * oc) - (size * size);

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

		//float radius;

	private:
		static constexpr const char name[] = "Sphere";
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

		void setIntensity(float i)
		{
			intensity = i;
		}

		float getIntensity()
		{
			return intensity;
		}

		virtual void printProperties() override
		{
			SceneObject::printProperties();
			std::cout << "size: " << size << std::endl;
			std::cout << "intensity: " << intensity << std::endl;
		}

		float size;
		float intensity;

	private:
		LightType lightType;
		
};

class PointLightObject : public LightObject {
	public:
		PointLightObject(float i) : LightObject(LightType::POINT) { LightObject::setSize(1.0f); }

		std::string_view getObjectName() override
		{
			return name;
		}

		virtual void printProperties() override
		{
			LightObject::printProperties();
			std::cout << "Type: " << getObjectName() << std::endl;
		}

	private:
		static constexpr const char name[] = "Point Light";
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
			std::cout << "look at: " << get_lookAt().x << " " << get_lookAt().y << " " << get_lookAt().z << std::endl;
		}

		Vector3D<float> lookAt;
		
	private:
		CameraType cameraType;
};

class PerspectiveCameraObject : public CameraObject {
	public:
		PerspectiveCameraObject(float fov) : CameraObject(CameraType::PERSPECTIVE), fieldOfView(fov) {}

		std::string_view getObjectName() override
		{
			return name;
		}

		virtual void printProperties() override
		{
			CameraObject::printProperties();
			std::cout << "Type: " << getObjectName() << std::endl;
		}

	private:
		static constexpr const char name[] = "Camera Perspective";
		float fieldOfView;
};

bool SceneBuilder(const std::string&, std::vector<SceneObject*>&);

void tokenSearch(std::ifstream& file, char c, std::string& token);
void setObjectParameters(std::ifstream& file, std::string& token, std::vector<SceneObject*>& sceneObjects);