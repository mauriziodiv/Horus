#pragma once
#include "RealObj.h"
#include "hrs.h"
#include "triangle.h"
#include <filesystem>
#include <algorithm>
#include <cctype>

struct LightSample
{
	Vector3D<float> position;
	Vector3D<float> normal;
	float pdf;
};

class Mesh : public GeometryObject
{

	public:

		Mesh();

		//bool importObj(const char* filename);
		//bool getData();
		//void buildTriangles();

		std::string_view getObjectName() override { return name; }

		virtual bool loadGeometry(const char* filename) override;
		virtual std::vector <GeometryObject*> getTriangles() override;

	private:
		ObjMesh objMesh;
		TriangleMesh triangleMesh;
		std::vector<Triangle> triangles;

		Vector3D<float> VecToVector(Vec3D v);

		static constexpr const char name[] = "Mesh";

		bool getData();
		void buildTriangles();

		bool importObj(const char* filename);

	protected:
		const std::vector<Triangle>& getTrianglesList() const { return triangles; };
};

class MeshLight : public Mesh
{
	public:

		MeshLight() : intensity(0.0f), exposure(0.0f), color(1.0f, 1.0f, 1.0f), totalArea(0.0f) {}

		void setIntensity(float i) { intensity = i; }
		void setExposure(float e) { exposure = e; }
		void setColor(const Vector3D<float>& c) { color = c; }

		float getIntensity() { return intensity; }
		float getExposure() { return exposure; }
		Vector3D<float> getColor() { return color; }

		void buildCDF();
		LightSample sampleLight(float rndTriangle, float rU, float rV) const;

	private:
		float intensity;
		float exposure;
		Vector3D<float> color;
		std::vector<float> cdf;
		float totalArea;
};