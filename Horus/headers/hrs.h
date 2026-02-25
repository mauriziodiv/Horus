#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <string_view>
#include <variant>
#include "vec_math.h"
#include "ray.h"
#include "shader.h"

struct BoundingBox
{
	public:
		BoundingBox() : min(0.0f, 0.0f, 0.0f), max(0.0f, 0.0f, 0.0f) {}
		BoundingBox(const Vector3D<float>& mn, const Vector3D<float>& mx) : min(mn), max(mx) {}

		void setMin(const Vector3D<float> mn) { min = mn; }
		void setMax(const Vector3D<float> mx) { max = mx; }

		Vector3D<float> getMin() const { return min; }
		Vector3D<float> getMax() const { return max; }

	private:
		Vector3D<float> min;
		Vector3D<float> max;
};

enum class ShaderType {
	CONSTANT
};

enum class ShaderParameterType {
	COLOR
};

struct HitRecord
{
	bool front = false;
	bool back = false;

	Vector3D<float> hitPoint;
	float t = 0.0f;
};

enum class ParameterType {
	POSITION,
	ROTATION,
	SIZE,
	WIDTH,
	HEIGHT,
	RADIUS,
	INTENSITY,
	LAT,
	WINDOW,
	SHADER
};

extern std::unordered_map<std::string, ParameterType> parameterMap;

enum class SceneObjectType {
	GEOMETRY,
	LIGHT,
	CAMERA
};

enum class GeometryType {
	SPHERE,
	PLANE
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

		SceneObject(SceneObjectType t) : type(t), position(0.0f, 0.0f, 0.0f), RIGHT(1.0f, 0.0f, 0.0f), UP(0.0f, 1.0f, 0.0f), FORWARD(0.0f, 0.0f, -1.0f) {}

		SceneObjectType getType()
		{
			return type;
		}

		virtual void printProperties()
		{
			std::cout << "position: " << position.x << " " << position.y << " " << position.z << std::endl;
		}

		virtual float getIntensity() { return 0.0f; };
		
		virtual std::string_view getObjectName() = 0;

		Vector3D<float> position;
		Vector3D<float> rotation;

	private:
		SceneObjectType type;

		Vector3D<float> RIGHT;
		Vector3D<float> UP;
		Vector3D<float> FORWARD;
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

		virtual void printProperties() override
		{
			SceneObject::printProperties();
		}

		virtual void setBoundingBox() {}

		virtual void computeNormal() {}

		void setPositionUpdated(bool updated) { positionUpdated = updated; }
		void setRotationUpdated(bool updated) { rotationUpdated = updated; }
		void setWidthUpdated(bool updated) { widthUpdated = updated; }
		void setHeightUpdated(bool updated) { heightUpdated = updated; }

		bool getPositionUpdated() { return positionUpdated; }
		bool getRotationUpdated() { return rotationUpdated; }
		bool getWidthUpdated() { return widthUpdated; }
		bool getHeightUpdated() { return heightUpdated; }

		bool checkPositionRotationWidthHeightUpdated() { return positionUpdated && rotationUpdated && widthUpdated && heightUpdated; }

		virtual Vector3D<float> getNormal() { return Vector3D<float>(0.0f, 0.0f, 0.0f); };

		virtual bool rayIntersection(Ray& ray, float tMin, float tMax) { return false; };

		bool linkShader(std::string& shaderFilePath);

		std::variant<Shader, Constant>& getShader() { return shader; }

		bool getHitRecordFront() { return hitRecord.front; }
		bool getHitRecordBack() { return hitRecord.back; }

		float size;
		HitRecord hitRecord;

		BoundingBox boundingBox;

	private:

		GeometryType geometryType;
		std::ifstream shaderFile;

		std::string token;

		std::variant<Shader, Constant> shader;

		bool parse();

		bool assignShader(ShaderType sType);

		bool positionUpdated = false;
		bool rotationUpdated = false;
		bool widthUpdated = false;
		bool heightUpdated = false;
};

class SphereObject : public GeometryObject {

	public:
		SphereObject(float r = 1.0f) : GeometryObject(GeometryType::SPHERE) { GeometryObject::size = r; setBoundingBox(); }
		
		std::string_view getObjectName() override
		{
			return name;
		}

		virtual void setBoundingBox() override
		{
			Vector3D<float> mn = Vector3D<float>(position.x - size, position.y - size, position.z - size);
			Vector3D<float> mx = Vector3D<float>(position.x + size, position.y + size, position.z + size);

			boundingBox.setMin(mn);
			boundingBox.setMax(mx);
		}

		virtual void printProperties() override
		{
			GeometryObject::printProperties();
			std::cout << "Type: " << getObjectName() << std::endl;
			std::cout << "radius: " << size << std::endl;
		}

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

	private:
		static constexpr const char name[] = "Sphere";
};

class PlaneObject : public GeometryObject {

	public:

		PlaneObject();

		std::string_view getObjectName() override
		{
			return name;
		}

		void setWidth(float w) { width = w; }
		void setHeight(float h) { height = h; }

		float getWidth() { return width; }
		float getHeight() { return height; }

		virtual void computeNormal() override;

		virtual Vector3D<float> getNormal() override
		{
			return normal;
		}

		virtual bool rayIntersection(Ray& ray, float tMin, float tMax) override
		{
			if (ray.direction * normal == 0.0f) { return false; }

			float t = ((position - ray.getOrigin()) * normal) / (ray.getDirection() * normal);

			if (t > 0 && t> tMin && t < tMax)
			{
				Vector3D<float> hitPoint = ray.getPointat(t);

				if (hitPoint > min && hitPoint < max)
				{
					hitRecord.front = true;
					hitRecord.back = false;
					hitRecord.hitPoint = hitPoint;
					hitRecord.t = t;

					return true;
				}
			}

			return false;
		}

	private:

		float width;
		float height;

		Vector3D<float> min;
		Vector3D<float> max;

		Vector3D<float> normal;

		static constexpr const char name[] = "Plane";

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
			position = Vector3D<float>(x, y, z);
		}

		Vector3D<float> getPosition()
		{
			return position;
		}

		void setWindow(float w, float h) 
		{ 
			width = w;
			height = h;

			aspect_ratio = width / height;
			window_height = 2.0f;
			window_width = window_height * aspect_ratio;

			Matrix4X4<float> R = Matrix4X4<float>::RotationY(rotation.y * DegreeToRadians) * Matrix4X4<float>::RotationX(rotation.x * DegreeToRadians) * Matrix4X4<float>::RotationZ(rotation.z * DegreeToRadians);

			Vector3D<float> forward = R * Vector3D<float>(0.0f, 0.0f, -1.0f);
			Vector3D<float> right = R * Vector3D<float>(1.0f, 0.0f, 0.0f);
			Vector3D<float> up = R * Vector3D<float>(0.0f, 1.0f, 0.0f);

			lower_left_corner = position + (forward * focal_length) - (right * window_width * 0.5) - (up * window_height * 0.5);
			horizontal = right * window_width;
			vertical = up * window_height;
		}

		float getWidth() { return width; }
		float getHeight() { return height; }

		Ray genRay(float u, float v); 
		

		Vector3D<float> lookAt;
		
	private:
		CameraType cameraType;

		float width;
		float height;

		float aspect_ratio;
		float window_height;
		float window_width;

		float focal_length;

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
void charSearch(std::ifstream& file, char c, std::string& token);
void setObjectParameters(std::ifstream& file, std::string& token, std::vector<SceneObject*>& sceneObjects);