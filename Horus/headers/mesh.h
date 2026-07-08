#pragma once
#include "RealObj.h"
#include "hrs.h"
#include "triangle.h"
#include <filesystem>
#include <algorithm>
#include <cctype>

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
};