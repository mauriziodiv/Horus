#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <string_view>
#include "vec_math.h"
#include "ray.h"

struct HitRecord
{
	bool front = false;

	Vector3D<float> hitPoint;
	float t = 0.0f;
};

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

		virtual bool rayIntersection(Ray& ray, float tMin, float tMax) { return false; };

		bool getHitRecordFront() { return hitRecord.front; }
		bool getHitRecordBack() { return hitRecord.front; }

		float size;
		HitRecord hitRecord;

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

		//bool getHitRecordFront() { return hitRecord.front; }
		//bool getHitRecordBack() { return hitRecord.front; }

		// Implements ray-sphere intersection using the quadratic formula.
		bool rayIntersection(Ray& ray, float tMin, float tMax) override
		{
			float a = ray.direction * ray.direction;

			Vector3D<float> oc = ray.origin - position;

			float b = ((ray.direction) * oc) * 2.0f;

			float c = (oc * oc) - (size * size);

			float discriminant = (b * b) - (4 * a * c);

			if (discriminant < 0)
			{
				return false;
			}

			float sqr = std::sqrt(discriminant);

			float t1 = (-b - sqr) / (2.0f * a);
			float t2 = (-b + sqr) / (2.0f * a);

			if (t1 > tMin && t1 < tMax)
			{
				hitRecord.front = true;
				hitRecord.hitPoint = ray.getPointat(t1);
				hitRecord.t = t1;
				return true;
			}

			if (t2 > tMin && t2 < tMax)
			{
				hitRecord.front = false;
				hitRecord.hitPoint = ray.getPointat(t2);
				hitRecord.t = t2;

				return true;
			}
		
			return false;
		}

		//float radius;

	private:
		static constexpr const char name[] = "Sphere";
		//HitRecord hitRecord;
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
		CameraObject(CameraType cType) : SceneObject(SceneObjectType::CAMERA), cameraType(cType), lookAt(Vector3D<float>(0.0f, 0.0f, 0.0f)), focal_length(1.0f), width(200.0f), height(100.0f)
		{
			
		}

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

		void setPosition(float x, float y, float z)
		{
			//position = Vector3D<float>(x, y, z);
			//camera.setPosition(position);
			position = Vector3D<float>(x, y, z);
		}

		Vector3D<float> getPosition()
		{
			return position;
		}

		void setWindow(float w, float h) 
		{ 
			//camera.setWindow(width,  height);

			width = w;
			height = h;

			aspect_ratio = width / height;
			window_height = 2.0f;
			window_width = window_height * aspect_ratio;

			lower_left_corner = Vector3D<float>(position.x - (window_width * 0.5), position.y - (window_height * 0.5), position.z - focal_length);
			horizontal = Vector3D<float>(window_width, 0.0f, 0.0f);
			vertical = Vector3D<float>(0.0f, window_height, 0.0f);
		}

		float getWidth() { return width; }
		float getHeight() { return height; }

		Ray genRay(float u, float v); 
		

		Vector3D<float> lookAt;
		
	private:
		CameraType cameraType;
		//Camera camera;

		float width;
		float height;

		float aspect_ratio;
		float window_height;
		float window_width;

		float focal_length;

		//Vector3D<float> position;

		Vector3D<float> lower_left_corner;
		Vector3D<float> horizontal;
		Vector3D<float> vertical;
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