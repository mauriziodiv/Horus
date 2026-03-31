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
#include "BxDF.h"

struct Morton
{
	uint32_t code;
};

inline uint32_t expandBits(uint32_t val)
{
	if (val >= (1 << 10))
	{
		val = (1 << 10) - 1;
	}

	val = (val | (val << 16)) & 0b00000011000000000000000011111111;
	val = (val | (val << 8)) & 0b00000011000000001111000000001111;
	val = (val | (val << 4)) & 0b00000011000011000011000011000011;
	val = (val | (val << 2)) & 0b00001001001001001001001001001001;

	return val;
}

inline uint32_t computeMorton(Vector3D<float> c)
{
	uint32_t cX32 = static_cast<uint32_t>(c.x);
	uint32_t cY32 = static_cast<uint32_t>(c.y);
	uint32_t cZ32 = static_cast<uint32_t>(c.z);

	return expandBits(cZ32) << 2 | expandBits(cY32) << 1 | expandBits(cX32);
}

struct BoundingBox
{
	public:
		BoundingBox() : min(0.0f, 0.0f, 0.0f), max(0.0f, 0.0f, 0.0f) {}
		BoundingBox(const Vector3D<float>& mn, const Vector3D<float>& mx) : min(mn), max(mx) {}

		void setMin(const Vector3D<float> mn) { min = mn; }
		void setMax(const Vector3D<float> mx) { max = mx; }

		void assignBoundingBox(const BoundingBox& bb)
		{
			min = bb.getMin();
			max = bb.getMax();
		}

		Vector3D<float> getMin() const { return min; }
		Vector3D<float> getMax() const { return max; }

		void computeCentroid() { centroid = (min * 0.5f) + (max * 0.5f); }
		Vector3D<float> getCentroid() const { return centroid; }
		void setCentroid(Vector3D<float> c) { centroid = c; };

		bool intersect(const Ray& ray, float tMin, float tMax) const
		{
			for (int i = 0 ; i< 3; ++i)
			{
				float invDir = 1.0f / ray.getDirection()[i];

				float t0 = (min[i] - ray.getOrigin()[i]) * invDir;
				float t1 = (max[i] - ray.getOrigin()[i]) * invDir;

				if (invDir < 0.0f)
				{
					std::swap(t0, t1);
				}

				if (t0 > tMin) { tMin = t0; }
				if (t1 < tMax) { tMax = t1; }

				if (tMax <= tMin) { return false; }
			}
			return tMax > tMin;
		}

		float getSurfaceArea()
		{
			float A1 = (max.x - min.x) * (max.y - min.y);
			float A2 = (max.z - min.z) * (max.y - min.y);
			float A3 = (max.x - min.x) * (max.z - min.z);

			return 2.0f * (A1 + A2 + A3);
		}

		BoundingBox& operator+=(const BoundingBox& a)
		{
			float minX = std::min(a.getMin().x, min.x);
			float minY = std::min(a.getMin().y, min.y);
			float minZ = std::min(a.getMin().z, min.z);

			float maxX = std::max(a.getMax().x, max.x);
			float maxY = std::max(a.getMax().y, max.y);
			float maxZ = std::max(a.getMax().z, max.z);

			min = Vector3D<float>(minX, minY, minZ);
			max = Vector3D<float>(maxX, maxY, maxZ);

			return *this;
		}

	private:
		Vector3D<float> min;
		Vector3D<float> max;

		Vector3D<float> centroid;
};

inline BoundingBox operator+(const BoundingBox& a, const BoundingBox& b)
{
	BoundingBox c = a;
	return c += b;
}

enum class ShaderType {
	CONSTANT,
	DEPTH,
	SURFACE
};

enum class ShaderParameterType {
	COLOR,
	DIFFUSE_GAIN,
	DIFFUSE_COLOR,
	ROUGHNESS,
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
	COLOR,
	DIFFUSE_GAIN,
	DIFFUSE_COLOR,
	ROUGHNESS,
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
	POINT,
	DOME
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
		BoundingBox& getBoundingBox() { return boundingBox; }

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

		std::variant<Shader, Constant, Depth, Surface>& getShader() { return shader; }

		bool getHitRecordFront() { return hitRecord.front; }
		bool getHitRecordBack() { return hitRecord.back; }

		void createMorton() { morton.code = computeMorton(boundingBox.getCentroid()); }

		Morton getMorton() { return morton; }

		float size;
		HitRecord hitRecord;

		BoundingBox boundingBox;

	private:

		GeometryType geometryType;
		std::ifstream shaderFile;

		std::string token;

		std::variant<Shader, Constant, Depth, Surface> shader;

		bool parse();

		bool assignShader(ShaderType sType);

		bool positionUpdated = false;
		bool rotationUpdated = false;
		bool widthUpdated = false;
		bool heightUpdated = false;

		Morton morton;
};

class SphereObject : public GeometryObject {

	public:

		SphereObject(float r = 1.0f) : GeometryObject(GeometryType::SPHERE), normal(0.0f, 0.0f, 0.0f) { GeometryObject::size = r; setBoundingBox(); }
		
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

			boundingBox.computeCentroid();
		}

		virtual void computeNormal() override
		{
			normal = hitRecord.hitPoint - position;
			normal.normalize();
		}

		virtual Vector3D<float> getNormal() override
		{
			return normal;
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

		Vector3D<float> normal;

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

		virtual void setBoundingBox() override
		{
			Matrix4X4<float> R = Matrix4X4<float>::RotationY(rotation.y * DegreeToRadians) * Matrix4X4<float>::RotationX(rotation.x * DegreeToRadians) * Matrix4X4<float>::RotationZ(rotation.z * DegreeToRadians);
			
			Vector3D<float> corners[4] = {
				Vector3D<float> (-width * 0.5, -height * 0.5, 0.0f),
				Vector3D<float>(width * 0.5, height * 0.5, 0.0f),
				Vector3D<float>(-width * 0.5, height * 0.5, 0.0f),
				Vector3D<float>(width * 0.5, -height * 0.5, 0.0f)
			};
			
			//Vector3D<float> pc(- width * 0.5, -height * 0.5, 0.0f);
			min = position + (R * corners[0]);
			max = min;

			for (size_t i = 1; i < 4; ++i)
			{
				Vector3D<float> corner = position + (R * corners[i]);
				if (corner.x < min.x) { min.x = corner.x; };
				if (corner.y < min.y) { min.y = corner.y; };
				if (corner.z < min.z) { min.z = corner.z; };
				if (corner.x > max.x) { max.x = corner.x; };
				if (corner.y > max.y) { max.y = corner.y; };
				if (corner.z > max.z) { max.z = corner.z; };
			}

			const float epsilon = 0.001f;
			boundingBox.setMin(min - Vector3D<float>(epsilon, epsilon, epsilon));
			boundingBox.setMax(max + Vector3D<float>(epsilon, epsilon, epsilon));

			boundingBox.computeCentroid();
		}

		virtual bool rayIntersection(Ray& ray, float tMin, float tMax) override
		{
			if (ray.direction * normal == 0.0f) { return false; }

			float t = ((position - ray.getOrigin()) * normal) / (ray.getDirection() * normal);

			if (t > 0 && t > tMin && t < tMax)
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

		LightObject(LightType lType) : SceneObject(SceneObjectType::LIGHT), lightType(lType), color(0.0f, 0.0f, 0.0f) {}

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

		void setColor(Vector3D<float> col)
		{
			color = col;
		}

		Vector3D<float> getColor()
		{
			return color;
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

		Vector3D<float> color;
		
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

class DomeLightObject : public LightObject {
	
	public:

		DomeLightObject() : LightObject(LightType::DOME) {  }

		std::string_view getObjectName() override
		{
			return name;
		}

	private:

		static constexpr const char name[] = "Dome Light";
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

bool SceneBuilder(const std::string&, std::vector<std::unique_ptr<SceneObject>>&);

void tokenSearch(std::ifstream& file, char c, std::string& token);
void charSearch(std::ifstream& file, char c, std::string& token);
void setObjectParameters(std::ifstream& file, std::string& token, std::vector<std::unique_ptr<SceneObject>>& sceneObjects);