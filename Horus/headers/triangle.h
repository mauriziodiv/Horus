#pragma once
#include <vector>
#include "vec_math.h"
#include "ray.h"
#include "hrs.h"

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

class Triangle : public GeometryObject
{
	public:
		Triangle();

		void findVertices(int mIndex, int tIndex);

		virtual bool rayIntersection(Ray &ray, float tMin, float tMax) override;

		int meshIndex;
		int triangleIndex;

		static std::vector<const TriangleMesh*> *allMeshes;

		Vector3D<float> vertices[3];

		virtual void setBoundingBox() override;
		virtual void computeNormal() override;
		virtual Vector3D<float> getNormal() override;

		std::string_view getObjectName() override { return name; }

	private:
		float epsilon = 0.001f;
		
		Vector3D<float> normal;
		static constexpr const char name[] = "Triangle";
};