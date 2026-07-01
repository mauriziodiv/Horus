#pragma once
#include "RealObj.h"
#include "hrs.h"
#include "triangle.h"

class Mesh : public GeometryObject
{

	public:

		Mesh();

		bool importObj(const char* filename);
		bool getData();
		void buildTriangles();

	private:
		ObjMesh objMesh;
		TriangleMesh triangleMesh;
		std::vector<Triangle> triangles;

		Vector3D<float> VecToVector(Vec3D v);
};