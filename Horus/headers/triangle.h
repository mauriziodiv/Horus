#pragma once
#include <vector>
#include "vec_math.h"
#include "ray.h"

class TriangleMesh
{
	public:
		
		int nTriangles;
		int nVertices;

		const int* vertexIndices = nullptr;

		const Vector3D<float> *vertex = nullptr;
		const Vector3D<float> *vertexNormal = nullptr;
		const Vector3D<float> *vertexUV = nullptr;

		bool reverseOrientation, transformSwapsHandedness;

	private:
};

class Triangle
{
	public:
		Triangle();

		void findVertices(int mIndex, int tIndex);

		bool rayIntersection(Ray &ray);

		int meshIndex;
		int triangleIndex;

		static std::vector<const TriangleMesh*> *allMeshes;

		Vector3D<float> vertices[3];

	private:
};